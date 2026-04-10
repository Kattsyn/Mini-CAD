@mainpage Мини-САПР — Документация

## Описание проекта

**Мини-САПР** — учебное приложение для 2D-черчения, реализованное на C++17 и Qt6 Widgets.
Демонстрирует ключевые архитектурные подходы САПР-систем:

- Геометрические примитивы с сериализацией
- Параметрическое моделирование (редактирование фигур через числовые параметры)
- История операций (паттерн Command + Undo/Redo)
- Загрузка/выгрузка инженерных данных (`.sapr` JSON, SVG-экспорт)
- Покрытие тестами (Qt Test)

---

## Архитектура

### Основные классы

| Класс | Файл | Назначение |
|-------|------|------------|
| MainWindow | mainwindow.h | Главное окно, меню, тулбар, dock истории |
| DrawingScene | drawingscene.h | Сцена: создание фигур, отслеживание перемещений |
| DrawingView | drawingscene.h | Вьюпорт: зум колёсиком, удаление клавишей |
| LineItem | shapes.h | Отрезок с JSON-сериализацией |
| CircleItem | shapes.h | Эллипс/окружность с JSON-сериализацией |
| RectItem | shapes.h | Прямоугольник с JSON-сериализацией |
| AddShapeCommand | commands.h | Undo-команда добавления фигуры |
| RemoveShapesCommand | commands.h | Undo-команда удаления фигур |
| MoveShapesCommand | commands.h | Undo-команда перемещения |
| EditShapeCommand | commands.h | Undo-команда параметрического редактирования |
| ShapeParamsDialog | shapedialog.h | Диалог числового ввода параметров фигуры |

### Поток данных

```
Действие пользователя
    → MainWindow (тулбар / меню)
    → DrawingScene (режим инструмента, перо)
    → QUndoStack (команда)
    → Shape-объекты (геометрия)
    → JSON-файл (сериализация)
```

---

## Сборка

```bash
cmake -B build -S .
cmake --build build --parallel 4
```

## Тесты

```bash
cd build && ctest --output-on-failure
```

## Формат файла `.sapr`

```json
{
  "shapes": [
    { "type": "line",   "x1": 0, "y1": 0, "x2": 100, "y2": 100,
      "pen": { "color": "#000000", "width": 2 } },
    { "type": "circle", "x": 50, "y": 50, "w": 80, "h": 80,
      "pen": { "color": "#ff0000", "width": 1 } },
    { "type": "rect",   "x": 10, "y": 10, "w": 200, "h": 150,
      "pen": { "color": "#0000ff", "width": 3 } }
  ]
}
```
