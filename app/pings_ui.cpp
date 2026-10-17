#include <iostream>
#include <limits>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <memory>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
// #include <ftxui/ftxui.hpp>

using namespace ftxui;

namespace ui
{
    int ui()
    {
        auto screen = ScreenInteractive::Fullscreen();

        // --- Состояние приложения ---
        int hp = 100;
        int event_counter = 0;
        std::vector<std::string> log_lines;
        std::atomic<bool> background_work = true;
        bool show_log = true; // для демонстрации Maybe()

        // --- Стиль для кнопок (версия 6.1.9) ---
        auto button_style = ButtonOption::Animated();
        button_style.transform = [](const EntryState &s)
        {
            auto element = text(s.label);
            if (s.focused)
            {
                element |= bold | color(Color::Cyan1);
            }
            return element | center | borderEmpty | flex;
        };

        // --- Компоненты ---
        auto hit_button = Button("Hit (-1 HPP)", [&]
                                 { hp -= 1; }, button_style);
        auto hard_button = Button("Hard (-10 HP)", [&]
                                  { hp -= 10; }, button_style);
        auto heal_button = Button("Heal (+10 HP)", [&]
                                  { hp += 10; }, button_style);

        auto buttons = Container::Horizontal({
            hit_button,
            hard_button,
            heal_button,
        });

        // --- Компонент лога (прокручиваемый) ---
        auto log_component = Renderer([&]
                                      {
        Elements log_elements;
        for (const auto& line : log_lines) {
            log_elements.push_back(text(line));
        }
        return vbox(log_elements) | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10); });

        // --- Компонент, который можно скрывать/показывать через Maybe() ---
        auto maybe_log = Maybe(log_component, &show_log);

        // --- Главный компонент ---
        // ВАЖНО: используем Renderer с дочерним компонентом, чтобы события передавались [citation:6]
        auto main_component = Renderer(buttons, [&]
                                       {
        auto status_text = hbox({
            text(" HP: ") | bold,
            gauge(hp / 100.0f) | color(Color::Green),
            text(" " + std::to_string(hp) + "/100 "),
            text(" Events: " + std::to_string(event_counter)),
            text(" [l:toggle log]"),
        });

        return vbox({
            status_text | border,
            separator(),
            text(" Log:") | bold,
            maybe_log->Render(),
            separator(),
            buttons->Render() | center,
        }) | border; });

        // --- Обработка событий через CatchEvent (основной способ в 6.1.9) ---
        main_component |= CatchEvent([&](Event event)
                                     {
                                         // Выход по 'q'
                                         if (event == Event::Character('q'))
                                         {
                                             screen.ExitLoopClosure()();
                                             return true;
                                         }
                                         // Переключение видимости лога по 'l'
                                         if (event == Event::Character('l'))
                                         {
                                             show_log = !show_log;
                                             return true; // событие обработано
                                         }
                                         // Очистка лога по Ctrl+L
                                         if (event == Event::CtrlL)
                                         {
                                             log_lines.clear();
                                             log_lines.push_back("--- Log cleared ---");
                                             return true;
                                         }
                                         return false; // событие не обработано, передаём дальше
                                     });

        // --- Фоновый поток для обновления UI ---
        std::thread background_thread([&]
                                      {
        int counter = 0;
        while (background_work) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            counter++;
            std::string msg = "[INFO] Background task #" + std::to_string(counter);
            log_lines.push_back(msg);
            if (log_lines.size() > 20) {
                log_lines.erase(log_lines.begin());
            }
            // Отправляем событие для перерисовки
            screen.PostEvent(Event::Custom);
        } });

        // --- Запуск ---
        screen.Loop(main_component);

        // --- Очистка ---
        background_work = false;
        if (background_thread.joinable())
        {
            background_thread.join();
        }

        return 0;
    }
}
