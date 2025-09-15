//
// Created by Андрей Подвысоцкий on 10.09.2025.
//

#include "hamurabbi.h"

#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <limits>
#include <iomanip>

struct State {
    int year = 1;                 // 1..10
    int population = 100;         // люди
    double wheat = 2800.0;        // бушели (дробные разрешены для точности семян/крыс)
    int acres = 1000;             // акры
    // Параметры отчёта за прошлый год:
    int starved_last = 0;
    int arrived_last = 0;
    bool plague_last = false;
    int harvested_last = 0;
    int yield_per_acre_last = 0;
    int rats_ate_last = 0;
    int price_last = 0;

    // Для финальной оценки
    int total_starved = 0;
    int total_population_at_start = 0; // сумма населения на начало каждого года (для среднего процента)
};

static const char* SAVE_FILE = "hammurabi_save.txt";

bool load(State& s) {
    std::ifstream in(SAVE_FILE);
    if (!in) return false;
    State tmp;
    in >> tmp.year
       >> tmp.population
       >> tmp.wheat
       >> tmp.acres
       >> tmp.starved_last
       >> tmp.arrived_last
       >> tmp.plague_last
       >> tmp.harvested_last
       >> tmp.yield_per_acre_last
       >> tmp.rats_ate_last
       >> tmp.price_last
       >> tmp.total_starved
       >> tmp.total_population_at_start;
    if (!in.fail()) {
        s = tmp;
        return true;
    }
    return false;
}

bool save(const State& s) {
    std::ofstream out(SAVE_FILE, std::ios::trunc);
    if (!out) return false;
    out << s.year << "\n"
        << s.population << "\n"
        << std::fixed << std::setprecision(6) << s.wheat << "\n"
        << s.acres << "\n"
        << s.starved_last << "\n"
        << s.arrived_last << "\n"
        << (s.plague_last ? 1 : 0) << "\n"
        << s.harvested_last << "\n"
        << s.yield_per_acre_last << "\n"
        << s.rats_ate_last << "\n"
        << s.price_last << "\n"
        << s.total_starved << "\n"
        << s.total_population_at_start << "\n";
    return out.good();
}

void discardLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int askInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        long long v;
        if (std::cin >> v) return static_cast<int>(v);
        std::cout << "Некорректный ввод. Введите целое число.\n";
        discardLine();
    }
}

char askChar(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        char c;
        if (std::cin >> c) return c;
        std::cout << "Некорректный ввод.\n";
        discardLine();
    }
}

bool askYesNo(const std::string& q) {
    while (true) {
        std::cout << q << " [y/n]: ";
        char c; if (!(std::cin >> c)) { discardLine(); continue; }
        if (c=='y' || c=='Y') return true;
        if (c=='n' || c=='N') return false;
        std::cout << "Ответьте 'y' или 'n'.\n";
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // RNG
    std::random_device rd;
    std::mt19937 rng(rd());

    State st;

    // Загрузка сохранения
    {
        State tmp;
        if (load(tmp)) {
            std::cout << "Найден сохранённый прогресс.\n";
            if (askYesNo("Продолжить предыдущую игру?")) {
                st = tmp;
            } else {
                // начать заново: перезапишем файл в первом сохранении
                st = State{};
            }
        } else {
            st = State{};
        }
    }

    // Главный цикл лет (раундов)
    while (true) {
        // Условия окончания (если уже дошли до 11 года — значит было сыграно 10)
        if (st.year > 10) break;

        // Случайная цена земли в начале раунда
        std::uniform_int_distribution<int> priceDist(17, 26);
        int price = priceDist(rng);
        st.price_last = price;

        // ВЫХОД и АВТОСОХРАНЕНИЕ в начале раунда
        std::cout << "\n=== ГОД " << st.year << " ===\n";
        std::cout << "Нажмите 'q' чтобы выйти и сохранить игру, или любую другую клавишу для продолжения: ";
        char c; std::cin >> c;
        if (!std::cin) { discardLine(); c = 0; }
        if (c=='q' || c=='Q') {
            if (save(st)) {
                std::cout << "Игра сохранена в \"" << SAVE_FILE << "\". До встречи!\n";
            } else {
                std::cout << "Не удалось сохранить игру. Проверьте права на запись.\n";
            }
            return 0;
        }

        // Сохранение снимка перед входом в решения (на случай внезапного выхода)
        if (!save(st)) {
            std::cout << "ВНИМАНИЕ: Не удалось автосохраниться. Продолжаем без гарантии сохранения.\n";
        }

        // ОТЧЁТ советника за прошлый год
        std::cout << "\nМой повелитель, соизволь поведать тебе\n";
        std::cout << "в году " << st.year << " твоего высочайшего правления\n";
        if (st.starved_last > 0)
            std::cout << st.starved_last << " человек умерли с голоду";
        if (st.starved_last > 0 && st.arrived_last > 0) std::cout << ", и ";
        if (st.arrived_last > 0)
            std::cout << st.arrived_last << " человек прибыли в наш великий город";
        if (st.starved_last > 0 || st.arrived_last > 0) std::cout << ";\n";
        if (st.plague_last)
            std::cout << "Чума уничтожила половину населения;\n";

        std::cout << "Население города сейчас составляет " << st.population << " человек;\n";
        if (st.harvested_last > 0) {
            std::cout << "Мы собрали " << st.harvested_last
                      << " бушелей пшеницы, по " << st.yield_per_acre_last
                      << " бушеля(ей) с акра;\n";
        }
        if (st.rats_ate_last > 0) {
            std::cout << "Крысы истребили " << st.rats_ate_last
                      << " бушелей пшеницы, оставив "
                      << static_cast<long long>(st.wheat + 0.0001) << " бушелей в амбарах;\n";
        }
        std::cout << "Город сейчас занимает " << st.acres << " акров;\n";
        std::cout << "1 акр земли стоит сейчас " << price << " бушелей.\n";

        // Ввод решений
        // Покупка/продажа земли и еда — целые, посев — целые акры
        int buy = 0, sell = 0, eat = 0, plant = 0;

        // Покупка
        while (true) {
            buy = askInt("Сколько акров земли повелеваешь купить? ");
            if (buy < 0) {
                std::cout << "Нельзя купить отрицательное количество акров.\n";
                continue;
            }
            double cost = static_cast<double>(buy) * price;
            if (cost > st.wheat + 1e-9) {
                std::cout << "О, повелитель, пощади нас! У нас только "
                          << static_cast<long long>(st.wheat + 0.0001)
                          << " бушелей пшеницы.\n";
                continue;
            }
            break;
        }
        st.wheat -= static_cast<double>(buy) * price;
        st.acres += buy;

        // Продажа
        while (true) {
            sell = askInt("Сколько акров земли повелеваешь продать? ");
            if (sell < 0) {
                std::cout << "Нельзя продать отрицательное количество акров.\n";
                continue;
            }
            if (sell > st.acres) {
                std::cout << "Нельзя продать больше, чем у тебя есть (" << st.acres << ").\n";
                continue;
            }
            break;
        }
        st.wheat += static_cast<double>(sell) * price;
        st.acres -= sell;

        // Еда
        while (true) {
            eat = askInt("Сколько бушелей пшеницы повелеваешь съесть? ");
            if (eat < 0) {
                std::cout << "Нельзя съесть отрицательное количество.\n";
                continue;
            }
            if (eat > static_cast<long long>(st.wheat + 0.0001)) {
                std::cout << "О, повелитель, пощади нас! В амбарах "
                          << static_cast<long long>(st.wheat + 0.0001) << " бушелей.\n";
                continue;
            }
            break;
        }
        st.wheat -= eat;

        // Посев (семена 0.5 буш/акр; проверить лимиты: по людям, по акрам, по семенам)
        while (true) {
            plant = askInt("Сколько акров земли повелеваешь засеять? ");
            if (plant < 0) { std::cout << "Нельзя засеять отрицательное количество.\n"; continue; }
            if (plant > st.acres) {
                std::cout << "Нельзя засеять больше, чем имеешь акров (" << st.acres << ").\n";
                continue;
            }
            int maxByPeople = st.population * 10;
            if (plant > maxByPeople) {
                std::cout << "У нас только " << st.population << " человек, они могут обработать "
                          << maxByPeople << " акров.\n";
                continue;
            }
            double seedsNeeded = plant * 0.5; // может быть x.0 или x.5
            if (seedsNeeded > st.wheat + 1e-9) {
                std::cout << "О, повелитель, пощади нас! Не хватает зерна на семена. Нужно "
                          << static_cast<long long>(seedsNeeded + 0.0001)
                          << ", в амбарах "
                          << static_cast<long long>(st.wheat + 0.0001) << ".\n";
                continue;
            }
            break;
        }
        st.wheat -= plant * 0.5;

        // ====== ПЕРЕХОД К СЛЕДУЮЩЕМУ ГОДУ (симуляция) ======
        // Урожай (1..6 буш/акр)
        std::uniform_int_distribution<int> yieldDist(1, 6);
        int ypa = yieldDist(rng);
        long long harvested = static_cast<long long>(plant) * ypa;
        st.yield_per_acre_last = ypa;
        st.harvested_last = static_cast<int>(harvested);
        st.wheat += harvested;

        // Крысы (0 .. floor(0.07 * wheat))
        {
            long long stockInt = static_cast<long long>(st.wheat + 0.0001);
            long long maxRats = static_cast<long long>(stockInt * 0.07 + 0.0001);
            if (maxRats < 0) maxRats = 0;
            std::uniform_int_distribution<long long> ratsDist(0, maxRats);
            long long eaten = ratsDist(rng);
            st.rats_ate_last = static_cast<int>(eaten);
            st.wheat -= eaten;
            if (st.wheat < 0) st.wheat = 0;
        }

        // Потребление. Каждый житель требует 20 бушелей/год.
        int canFeed = eat / 20;
        int starved = 0;
        if (canFeed < st.population) {
            starved = st.population - canFeed;
            st.population = canFeed;
        }
        st.starved_last = starved;

        // Условие мгновенного проигрыша: >45% умерли за год
        if (st.population + starved > 0) {
            double diedShare = (st.population == 0 && starved > 0)
                               ? 1.0
                               : static_cast<double>(starved) / static_cast<double>(st.population + starved);
            if (diedShare > 0.45) {
                std::cout << "\nКатастрофа! За этот год умерло от голода более 45% населения.\n";
                // Итоговый отчёт:
                st.total_starved += starved;
                st.total_population_at_start += (st.population + starved);
                // Сброс года до 11, чтобы показать финальную оценку прямо сейчас
                st.year = 11;
                break;
            }
        }

        // Иммиграция: умерших/2 + (5 - ypa) * wheat/600 + 1, затем clamp [0..50]
        int arrived = 0;
        {
            double term = (5 - ypa) * (st.wheat) / 600.0 + 1.0;
            int calc = starved / 2 + static_cast<int>(term);
            if (calc < 0) calc = 0;
            if (calc > 50) calc = 50;
            arrived = calc;
        }
        st.arrived_last = arrived;
        st.population += arrived;

        // Чума с вероятностью 15%
        {
            std::uniform_real_distribution<double> uni(0.0, 1.0);
            bool plague = (uni(rng) < 0.15);
            st.plague_last = plague;
            if (plague) {
                st.population = st.population / 2; // округление вниз
            }
        }

        // Для финальных метрик
        st.total_starved += starved;
        st.total_population_at_start += (st.population + starved); // население на начало года

        // Следующий год
        st.year += 1;

        // Автосохранение конца раунда
        if (!save(st)) {
            std::cout << "ВНИМАНИЕ: Не удалось сохранить прогресс после раунда.\n";
        }

        // Условия раннего конца
        if (st.population <= 0) {
            std::cout << "\nВсе жители вымерли. Правление окончено.\n";
            break;
        }
    }

    // === Финальная оценка, если сыграно >= 1 раунд ===
    // Защита от деления на ноль:
    double P = 0.0;
    if (st.total_population_at_start > 0) {
        // Средний годовой процент умерших = (суммарно умерли / сумма населения на старте лет) * 100
        P = 100.0 * static_cast<double>(st.total_starved) / static_cast<double>(st.total_population_at_start);
    }
    double L = (st.population > 0) ? (static_cast<double>(st.acres) / st.population) : 0.0;

    std::cout << "\n=== ИТОГ ПРАВЛЕНИЯ ===\n";
    std::cout << "Средний годовой процент умерших от голода P = " << std::fixed << std::setprecision(2) << P << "%\n";
    std::cout << "Акров земли на одного жителя L = " << std::fixed << std::setprecision(2) << L << "\n\n";

    std::string verdict;
    if (P > 33.0 && L < 7.0) {
        verdict = "Из-за вашей некомпетентности в управлении, народ устроил бунт, и изгнал вас из города. "
                  "Теперь вы вынуждены влачить жалкое существование в изгнании.";
    } else if (P > 10.0 && L < 9.0) {
        verdict = "Вы правили железной рукой, подобно Нерону и Ивану Грозному. "
                  "Народ вздохнул с облегчением, и никто больше не желает видеть вас правителем.";
    } else if (P > 3.0 && L < 10.0) {
        verdict = "Вы справились вполне неплохо, у вас, конечно, есть недоброжелатели, "
                  "но многие хотели бы увидеть вас во главе города снова.";
    } else {
        verdict = "Фантастика! Карл Великий, Дизраэли и Джефферсон вместе не справились бы лучше.";
    }
    std::cout << verdict << "\n";

    // После окончания уберём сохранение, чтобы новая игра начиналась «с нуля»
    std::remove(SAVE_FILE);

    return 0;
}
