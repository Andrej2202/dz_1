## Результаты бенчмарка умных указателей

### N = 10³ (1 000)

| Тип | N | Create + Del (ms) | Copy (ms) | Move (ms) | Access (ms) |
|:---|:---:|:---:|:---:|:---:|:---:|
| `raw new/delete` | 10³ | 0.06 | — | — | 0.00 |
| `my UnqPtr` | 10³ | 0.11 | — | 0.03 | 0.00 |
| `std::unique_ptr` | 10³ | 0.08 | — | 0.02 | 0.00 |
| `my ShrdPtr` | 10³ | 0.20 | 0.02 | — | 0.00 |
| `std::shared_ptr` | 10³ | 0.15 | 0.02 | — | 0.00 |

### N = 10⁶ (1 000 000)

| Тип | N | Create + Del (ms) | Copy (ms) | Move (ms) | Access (ms) |
|:---|:---:|:---:|:---:|:---:|:---:|
| `raw new/delete` | 10⁶ | 161.75 | — | — | 19.48 |
| `my UnqPtr` | 10⁶ | 147.34 | — | 58.75 | 18.13 |
| `std::unique_ptr` | 10⁶ | 154.01 | — | 110.19 | 19.42 |
| `my ShrdPtr` | 10⁶ | 276.69 | 13.68 | — | 16.86 |
| `std::shared_ptr` | 10⁶ | 124.41 | 17.82 | — | 17.86 |

STL shared быстрее за счет 1 аллокации вместо 2. У них контрол блок и птр обьеденены в 1 большой блок.
Сырые самые медленные за счет удаления в цикле.

## Проведённые функциональные тесты

### Тесты для `UnqPtr<T>` (уникальный указатель)

| № | Имя теста | Что проверяется |
|:---:|:---|:---|
| 1 | `test_unq_basic` | Базовое создание, доступ через `->`, `*`, метод `get()`, проверка `operator bool` |
| 2 | `test_unq_move` | Move-конструктор и move-присваивание, состояние источника после `std::move` |
| 3 | `test_unq_self_move` | Самоприсваивание через `std::move` (безопасность операции `p = std::move(p)`) |
| 4 | `test_unq_release_reset` | Методы `release()` (отвязка владения) и `reset()` (освобождение/замена объекта) |
| 5 | `test_unq_edge_cases` | Граничные случаи: пустой указатель, `reset(nullptr)`, `release()` из пустого |
| 6 | `test_unq_swap` | Метод `swap()` — обмен между двумя указателями, включая случай с пустыми |
| 7 | `test_unq_array` | Специализация для массивов `T[]`, доступ по индексу `arr[i]` |
| 8 | `test_unq_array_reset` | `reset()` для массивовой специализации |
| 9 | `test_unq_make` | Фабричные функции `make_unq<T>(...)` и `make_unq_arr<T>(...)` |
| 10 | `test_unq_subtype` | Подтипизация: `UnqPtr<Dog>` → `UnqPtr<Animal>` (move-семантика) |
| 11 | `test_unq_null` | Сравнение с `nullptr`, `operator==`, `operator!=`, приведение к `bool` |
| 12 | `test_unq_custom_deleter` | Пользовательский делитер (custom deleter) — вызов при уничтожении |
| 13 | `test_unq_get_deleter` | Доступ к делитеру через `get_deleter()` (const и non-const версии) |
| 14 | `test_unq_const_correctness` | Корректная работа с `const UnqPtr&` — нельзя менять указатель, но можно объект |
| 15 | `test_unq_move_only_type` | Работа с типами, у которых удалён копирующий конструктор (`MoveOnly`) |
| 16 | `test_unq_explicit_bool` | Явный `operator bool` в условиях `if (p)` / `if (!p)` |

### Тесты для `ShrdPtr<T>` (разделяемый указатель)

| № | Имя теста | Что проверяется |
|:---:|:---|:---|
| 1 | `test_shrd_basic` | Базовое создание, подсчёт ссылок `use_count()`, копирование |
| 2 | `test_shrd_move` | Move-конструктор и move-присваивание, `use_count` не меняется |
| 3 | `test_shrd_assignment` | Копирующее присваивание, освобождение старого объекта, `p = nullptr` |
| 4 | `test_shrd_self_assignment` | Самоприсваивание `p = p` — `use_count` не должен увеличиваться |
| 5 | `test_shrd_swap` | Метод `swap()` — обмен между указателями, корректность `use_count` |
| 6 | `test_shrd_complex` | Сложные сценарии: цепочки присваиваний `a = b; b = c; a = nullptr` |
| 7 | `test_shrd_subtype` | Подтипизация: `ShrdPtr<Dog>` → `ShrdPtr<Animal>` (копирование) |
| 8 | `test_shrd_array` | Специализация для массивов `T[]` с подсчётом ссылок |
| 9 | `test_shrd_make` | Фабричные функции `make_shrd<T>(...)` и `make_shrd_arr<T>(...)` |
| 10 | `test_shrd_null` | Сравнение с `nullptr`, пустой указатель, `use_count() == 0` |
| 11 | `test_shrd_custom_deleter` | Пользовательский делитер (lambda) — вызывается при `use_count == 0` |
| 12 | `test_shrd_const_correctness` | Корректная работа с `const ShrdPtr&` |
| 13 | `test_shrd_explicit_bool` | Явный `operator bool` в условиях `if (p)` / `if (!p)` |

### Дополнительные тесты

| № | Имя теста | Что проверяется |
|:---:|:---|:---|
| 1 | `test_circular_reference_demo` | Демонстрация проблемы циклических ссылок (`a->next = b; b->next = a`) — объекты **не уничтожаются**, что показывает необходимость `WeakPtr` |