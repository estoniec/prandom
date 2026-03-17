# gfrandom

[![CI](https://github.com/estoniec/prandom/actions/workflows/ci.yml/badge.svg)](https://github.com/estoniec/prandom/actions/workflows/ci.yml)
[![Clang-Tidy](https://img.shields.io/badge/Clang--Tidy-passing-green)](https://clang.llvm.org/docs/ClangFormat.html)

Linux kernel модуль, предоставляющий генератор псевдослучайных чисел на основе Galois Field (GF(2⁸)).

## Описание

Драйвер создаёт устройство `/dev/gfrandom`, которое возвращает псевдослучайные байты, сгенерированные с использованием арифметики Galois Field GF(2⁸). Является аналогом `/dev/urandom`.

### Алгоритм

Используется регистр сдвига с линейной обратной связью (LFSR) над полем GF(2⁸):
- Состояние: 256 байт (коэффициенты)
- Полином: t^256 + t^10 + t^5 + t^2 + 1 (или пользовательский)
- Генерируемое значение: сумма произведений ненулевых коэффициентов полинома на соответствующие коэффициенты состояния

### Используемые компоненты

- **gfrandom** — драйвер (GPL-3.0-or-later)
- **lib** — [библиотека арифметики Galois Field](https://github.com/DenisPotapov0/Galua-Field-library) (MIT License)

## Требования

- Linux kernel headers (для сборки модуля)
- GCC, make
- Для запуска тестов: linux-headers или root доступ для загрузки модуля

## Сборка

### Linux модуль

```bash
make
```

### Userspace тесты

```bash
make test_algo
./test
```

## Использование

### Загрузка модуля

```bash
sudo insmod gfrandom.ko
```

### Параметры модуля

- `coeff_file` — путь к файлу с коэффициентами (256 байт)
- `polynom_file` — путь к файлу с полиномом (256 байт)

Если параметры не указаны, используются автоматически сгенерированные значения.

Пример использования собственных параметров:
```bash
sudo insmod gfrandom.ko coeff_file=/path/to/coeffs.bin polynom_file=/path/to/poly.bin
```

### Чтение случайных данных

```bash
sudo cat /dev/gfrandom | head -c 1024 > /dev/null
```

### Выгрузка модуля

```bash
sudo rmmod gfrandom
```

## Тестирование

Тесты запускаются в CI и локально:

```bash
make test
```

## Лицензия

- **prandom**: GPL-3.0-or-later
- **lib** (Denis Potapov): MIT License

Подробности см. в [LICENSE](LICENSE).

## Автор

Макар Лилль