import random
import time
import threading

NUMBER_OF_BALLS = int(20)
DIAPAZON = 250
MIN_Y = 50
TIME = 1
TIMES = 3


class Ball:
    # Конструктор
    def __init__(self):
        # Поток для перемещения в цикле
        self.thread = threading.Thread(target=self.loop)
        # Поток для проверки
        self.checkThread = threading.Thread(target=self.checkLoop)
        # Блокировщик для синхронизации потоков
        self.lock = threading.Lock()

        # Координаты
        self.x = 0
        self.y = 0
        # Следует продолжать выполнение перемещений?
        self.should_continue = True

        # Запуск потоков
        self.thread.start()
        self.checkThread.start()

    # Перемещение
    def move(self):
        self.lock.acquire()
        self.y = random.randint(-DIAPAZON, DIAPAZON)
        self.x = random.randint(-DIAPAZON, DIAPAZON)

        time.sleep(TIME)

        # Чтобы не освобождать свободный блокировщик
        if self.lock.locked():
            self.lock.release()

        return self.should_continue

    # Проверка
    def check(self):
        self.lock.acquire()
        self.should_continue = self.y >= MIN_Y

        time.sleep(TIME)

        # Чтобы не освобождать свободный блокировщик
        if self.lock.locked():
            self.lock.release()

        return self.should_continue

    # Прекратить выполнение
    def stop(self):
        self.should_continue = False
        # Возвращает True, если блокировка получена
        if self.lock.locked():
            self.lock.release()

    # Основной поток
    def loop(self):
        while(self.move()):
            pass

    # Поток проверки
    def checkLoop(self):
        while(self.check()):
            pass

    # Деструктор
    def __del__(self):
        print("X:", self.x, ", Y:", self.y,
              (" - disappear" if self.y < MIN_Y else " - exists"))


# Список шаров
balls = []

# Наполнение списка объектами шаров
for i in range(NUMBER_OF_BALLS):
    balls.append(Ball())

# Ожидание
time.sleep(TIME * TIMES)

# Прекращение выполнения потоков
for ball in balls:
    ball.stop()
