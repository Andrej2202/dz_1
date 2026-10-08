# Список тестов

## UnqPtr

| № | Функция | Что проверяет |
|---|---|---|
| 1 | `test_unq_basic` | Базовое создание, `operator*`, `operator->`, `get()` |
| 2 | `test_unq_move` | Конструктор перемещения и `operator=` перемещения |
| 3 | `test_unq_self_move` | Самоприсваивание через `std::move` |
| 4 | `test_unq_release_reset` | Методы `release()` и `reset()` |
| 5 | `test_unq_edge_cases` | Граничные случаи: `reset()`/`release()` на nullptr |
| 6 | `test_unq_swap` | Метод `swap()`  |
| 7 | `test_unq_array` | Специализация для массивов `UnqPtr<T[]>`, `operator[]` |
| 8 | `test_unq_array_reset` | `reset()` для массивов |
| 9 | `test_unq_make` | Фабрики `make_unq` и `make_unq_arr` |
| 10 | `test_unq_subtype` | Подтипизация `UnqPtr<Dog>` -> `UnqPtr<Animal>` через move |
| 11 | `test_unq_null` | Работа с `nullptr`, операторы `==`/`!=` с nullptr |
| 12 | `test_unq_custom_deleter` | Кастомный делитель |
| 13 | `test_unq_get_deleter` | Метод `get_deleter()`  |
| 14 | `test_unq_const_correctness` | Const-корректность указателя и объекта |
| 15 | `test_unq_move_only_type` | Работа с move-only типами |
| 16 | `test_unq_explicit_bool` | `explicit operator bool()` |

## ShrdPtr

| № | Функция | Что проверяет |
|---|---|---|
| 17 | `test_shrd_basic` | Базовое создание, копирование, `use_count()` |
| 18 | `test_shrd_move` | Конструктор перемещения |
| 19 | `test_shrd_assignment` | Копирующее и move-присваивание, присваивание `nullptr` |
| 20 | `test_shrd_self_assignment` | Самоприсваивание `p = p` |
| 21 | `test_shrd_swap` | Метод `swap()` |
| 22 | `test_shrd_complex` | Сложные сценарии: множественное копирование, цепочка присваиваний |
| 23 | `test_shrd_subtype` | Подтипизация `ShrdPtr<Dog>` → `ShrdPtr<Animal>` |
| 24 | `test_shrd_array` | Специализация для массивов `ShrdPtr<T[]>` |
| 25 | `test_shrd_make` | Фабрики `make_shrd` и `make_shrd_arr` |
| 26 | `test_shrd_null` | Работа с `nullptr`, операторы `==`/`!=` с nullptr |
| 27 | `test_shrd_custom_deleter` | Кастомный делитель |   
| 28 | `test_shrd_const_correctness` | Const-корректность указателя и объекта |
| 29 | `test_shrd_explicit_bool` | `explicit operator bool()` |

## Демка

| № | Функция | Что проверяет |
|---|---|---|
| 30 | `test_circular_reference_demo` | Демка проблемы циклических ссылок |

---

### Итого
- **UnqPtr:** 16 тестов
- **ShrdPtr:** 13 тестов
- **Демка:** 1 тест
- **Всего:** 30 тестов