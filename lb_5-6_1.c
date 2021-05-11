#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

// Константы
constexpr unsigned int NUMBER = 10;
constexpr unsigned int MIN = -35;
constexpr unsigned int MAX = 200;
constexpr int MIN_Y = 30;

// Состояния шарика
const char *STATUS[] = {"exists", "disappiear"};

// Контроллер мьютекса для избежания ошибок
class timedMutex
{
public:
    void lock_for(unsigned int time)
    {
        if (!locked)
        {
            locked = t_mtx.try_lock_for(std::chrono::milliseconds(time));
            locked = false;
        }
    }

    void unlock()
    {
        if (locked)
        {
            t_mtx.unlock();
            locked = false;
        }
    }

private:
    std::timed_mutex t_mtx;
    bool locked = false;
};

// Конструктор шариков
class Ball
{
public:
    Ball(unsigned int number, unsigned int time)
    {
        num = number;  // Порядковый номер
        seed = time;  // Семя рандома (для большего рандома)

        thread = new std::thread(this->loop, this);  // Указатель на поток
    }
// Прерывание выполнения, через изменение переменной и разблокировку мьютекса (для прекращение задержки времени)
    void interrupt()
    {
        needs_interrupt = true;
        t_mtx.unlock();
    }

// Вернуть результат
    unsigned int getResult()
    {
        return result;
    }

    int coord_x()
    {
        return x;
    }

    int coord_y()
    {
        return y;
    }
    
// Деструктор
    ~Ball()
    {
        interrupt();
// Прервать поток, если они существует (проверка на нулевой указатель хорошая практика)
// Дождаться его завершения и освободить память
        if (thread != nullptr)
        {
            thread->join();
            delete thread;
        }
    }

private:
    int x = 0;
    int y = 0;
    unsigned int num = 0;
    unsigned int seed = 0;
    unsigned short result = 0;

// Указатели лучше инициализировать с нулевым указателем, чтобы потом можно было проверить на него
    std::thread *thread = nullptr;
    timedMutex t_mtx;
    bool needs_interrupt = false;

    bool move()
    {
        x = randomInt();
        y = randomInt();
        return y > MIN_Y;
    }

// Цикл выполнения движений, функция статическая чтобы было можно передать в поток
// Поэтому в качестве аргумента нужно передавать указатель на себя (this)
    static void loop(Ball *ptr)
    {
        while (ptr->move())
        {
// Мьютекс для ожидания использован чтобы было можно прервать его задержку
            ptr->t_mtx.lock_for(1000);

            if (ptr->needs_interrupt)
            {
                break;
            }
        }
        ptr->result = 1;
    }

// Чуть больше рандома богу рандома
    int randomInt()
    {
        srand(++seed);
        return MIN + rand() % (MAX + MIN);
    }
};

class BallsVector : public std::vector<Ball *>
{
public:
    explicit BallsVector(unsigned int size = 0) { this->reserve(size); };

    ~BallsVector()
    {
        std::cout << "Min y:" << MIN_Y << std::endl;
        for (int i = 0; i < this->size(); i++)
        {
            Ball *b = this->operator[](i);
            std::cout << " X:Y: "  << b->coord_x() << ":" << b->coord_y() << " - " << STATUS[b->getResult()] << std::endl;
            delete this->operator[](i);
        }
    }
};

int main()
{
    BallsVector balls(NUMBER);
    for (unsigned int i = 0; i < NUMBER; ++i)
    {
        balls.emplace_back(new Ball(i, i + time(0)));  // Поместить в назад в констурктор
    }

// Подождать три секунды
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
