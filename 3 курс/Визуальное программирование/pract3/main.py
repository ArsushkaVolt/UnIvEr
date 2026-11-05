import sys
import json
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QTableWidget, QTableWidgetItem, QPushButton, QLabel, QLineEdit,
    QComboBox, QTextEdit, QMessageBox, QFileDialog, QFormLayout, QDialog
)

# Начальный набор данных для демонстрации (сет Magic: The Gathering)
INITIAL_CARDS = [
    {
        "name": "Llanowar Elves",
        "type": "Creature",
        "color": "Green",
        "price": 0.50,
        "attack": 1,
        "health": 1,
        "text": "{T}: Add {G}.",
        "features": "-"
    },
    {
        "name": "Black Lotus",
        "type": "Artifact",
        "color": "Colorless",
        "price": 100000.0,
        "attack": 0,
        "health": 0,
        "text": "{T}, Sacrifice Black Lotus: Add three mana of any single color.",
        "features": "-"
    },
    {
        "name": "Jace, the Mind Sculptor",
        "type": "Planeswalker",
        "color": "Blue",
        "price": 45.0,
        "attack": 0,
        "health": 3,
        "text": "+2: Look at top card. 0: Draw 3 cards...",
        "features": "Loyalty: 3, Planeswalker - Jace"
    },
    {
        "name": "Lightning Bolt",
        "type": "Instant",
        "color": "Red",
        "price": 1.50,
        "attack": 0,
        "health": 0,
        "text": "Lightning Bolt deals 3 damage to any target.",
        "features": "-"
    }
]

class AddCardDialog(QDialog):
    """Диалоговое окно для добавления новой карты в базу."""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Добавить новую карту")
        self.resize(320, 380)

        layout = QFormLayout(self)

        self.name_input = QLineEdit()
        self.type_combo = QComboBox()
        self.type_combo.addItems(["Creature", "Instant", "Sorcery", "Artifact", "Enchantment", "Planeswalker", "Land"])
        self.color_combo = QComboBox()
        self.color_combo.addItems(["White", "Blue", "Black", "Red", "Green", "Colorless", "Multi-color"])
        self.price_input = QLineEdit("0.0")
        self.attack_input = QLineEdit("0")
        self.health_input = QLineEdit("0")
        self.text_input = QTextEdit()
        self.features_input = QLineEdit("-")

        layout.addRow("Название:", self.name_input)
        layout.addRow("Тип:", self.type_combo)
        layout.addRow("Цвет:", self.color_combo)
        layout.addRow("Цена ($):", self.price_input)
        layout.addRow("Атака (сущ.):", self.attack_input)
        layout.addRow("Здоровье (сущ.):", self.health_input)
        layout.addRow("Особенности:", self.features_input)
        layout.addRow("Текст карты:", self.text_input)

        btn_layout = QHBoxLayout()
        self.save_btn = QPushButton("Сохранить")
        self.cancel_btn = QPushButton("Отмена")
        btn_layout.addWidget(self.save_btn)
        btn_layout.addWidget(self.cancel_btn)

        layout.addRow(btn_layout)

        self.save_btn.clicked.connect(self.accept)
        self.cancel_btn.clicked.connect(self.reject)

    def get_card_data(self):
        try:
            price = float(self.price_input.text())
        except ValueError:
            price = 0.0
        try:
            attack = int(self.attack_input.text())
        except ValueError:
            attack = 0
        try:
            health = int(self.health_input.text())
        except ValueError:
            health = 0

        return {
            "name": self.name_input.text().strip() or "Unnamed",
            "type": self.type_combo.currentText(),
            "color": self.color_combo.currentText(),
            "price": price,
            "attack": attack,
            "health": health,
            "text": self.text_input.toPlainText().strip(),
            "features": self.features_input.text().strip()
        }

class MainWindow(QMainWindow):
    """Главное окно приложения базы карт MTG."""
    def __init__(self):
        super().__init__()
        self.setWindowTitle("База карт MTG")
        self.resize(950, 600)

        self.cards = INITIAL_CARDS.copy()

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # Панель фильтрации и поиска
        filter_layout = QHBoxLayout()

        filter_layout.addWidget(QLabel("Поиск по названию:"))
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("Введите имя карты...")
        self.search_input.textChanged.connect(self.filter_cards)
        filter_layout.addWidget(self.search_input)

        filter_layout.addWidget(QLabel("Фильтр по типу:"))
        self.type_filter = QComboBox()
        self.type_filter.addItem("Все типы")
        self.type_filter.addItems(["Creature", "Instant", "Sorcery", "Artifact", "Enchantment", "Planeswalker", "Land"])
        self.type_filter.currentIndexChanged.connect(self.filter_cards)
        filter_layout.addWidget(self.type_filter)

        main_layout.addLayout(filter_layout)

        # Таблица для вывода карт
        self.table = QTableWidget()
        self.table.setColumnCount(7)
        self.table.setHorizontalHeaderLabels([
            "Название", "Тип", "Цвет", "Цена ($)", "Атака / Здоровье", "Особенности", "Текст карты"
        ])
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)
        main_layout.addWidget(self.table)

        # Панель кнопок управления
        btn_layout = QHBoxLayout()

        self.btn_load = QPushButton("Загрузить из JSON")
        self.btn_load.clicked.connect(self.load_from_file)
        btn_layout.addWidget(self.btn_load)

        self.btn_save = QPushButton("Сохранить в JSON")
        self.btn_save.clicked.connect(self.save_to_file)
        btn_layout.addWidget(self.btn_save)

        self.btn_add = QPushButton("Добавить карту")
        self.btn_add.clicked.connect(self.add_card)
        btn_layout.addWidget(self.btn_add)

        self.btn_delete = QPushButton("Удалить выбранную")
        self.btn_delete.clicked.connect(self.delete_card)
        btn_layout.addWidget(self.btn_delete)

        main_layout.addLayout(btn_layout)

        # Первичное заполнение таблицы
        self.update_table(self.cards)

    def update_table(self, cards_to_show):
        """Обновление содержимого таблицы на основе переданного списка карт."""
        self.table.setRowCount(len(cards_to_show))
        for row, card in enumerate(cards_to_show):
            self.table.setItem(row, 0, QTableWidgetItem(card.get("name", "")))
            self.table.setItem(row, 1, QTableWidgetItem(card.get("type", "")))
            self.table.setItem(row, 2, QTableWidgetItem(card.get("color", "")))
            self.table.setItem(row, 3, QTableWidgetItem(str(card.get("price", 0.0))))

            # Форматирование характеристик атаки/здоровья в зависимости от типа
            card_type = card.get("type")
            if card_type == "Creature":
                stats = f"{card.get('attack', 0)} / {card.get('health', 0)}"
            elif card_type == "Planeswalker":
                stats = f"Loyalty: {card.get('health', 0)}"
            else:
                stats = "-"
            self.table.setItem(row, 4, QTableWidgetItem(stats))

            self.table.setItem(row, 5, QTableWidgetItem(card.get("features", "-")))
            self.table.setItem(row, 6, QTableWidgetItem(card.get("text", "")))

        self.table.resizeColumnsToContents()

    def filter_cards(self):
        """Фильтрация списка карт по поисковой строке и выбранному типу."""
        search_text = self.search_input.text().lower()
        selected_type = self.type_filter.currentText()

        filtered = []
        for card in self.cards:
            name_match = search_text in card.get("name", "").lower()
            type_match = (selected_type == "Все типы") or (card.get("type") == selected_type)

            if name_match and type_match:
                filtered.append(card)

        self.update_table(filtered)

    def add_card(self):
        """Добавление новой карты через диалоговое окно."""
        dialog = AddCardDialog(self)
        if dialog.exec() == QDialog.Accepted:
            new_card = dialog.get_card_data()
            self.cards.append(new_card)
            self.filter_cards()
            QMessageBox.information(self, "Успех", "Карта успешно добавлена в сет!")

    def delete_card(self):
        """Удаление выбранной карты из таблицы и общего списка."""
        selected_rows = self.table.selectionModel().selectedRows()
        if not selected_rows:
            QMessageBox.warning(self, "Внимание", "Пожалуйста, выберите строку с картой для удаления!")
            return

        row = selected_rows[0].row()
        card_name = self.table.item(row, 0).text()

        for card in self.cards:
            if card.get("name") == card_name:
                self.cards.remove(card)
                break

        self.filter_cards()
        QMessageBox.information(self, "Успех", f"Карта '{card_name}' была удалена.")

    def save_to_file(self):
        """Сериализация данных и сохранение сета в JSON файл."""
        file_path, _ = QFileDialog.getSaveFileName(self, "Сохранить сет в JSON", "mtg_set.json", "JSON Files (*.json)")
        if file_path:
            try:
                with open(file_path, "w", encoding="utf-8") as f:
                    json.dump(self.cards, f, ensure_ascii=False, indent=4)
                QMessageBox.information(self, "Успех", f"Данные успешно сохранены в файл:\n{file_path}")
            except Exception as e:
                QMessageBox.critical(self, "Ошибка", f"Не удалось сохранить файл:\n{e}")

    def load_from_file(self):
        """Десериализация данных и загрузка сета из JSON файла."""
        file_path, _ = QFileDialog.getOpenFileName(self, "Загрузить сет из JSON", "", "JSON Files (*.json)")
        if file_path:
            try:
                with open(file_path, "r", encoding="utf-8") as f:
                    self.cards = json.load(f)
                self.filter_cards()
                QMessageBox.information(self, "Успех", f"Данные успешно загружены из файла:\n{file_path}")
            except Exception as e:
                QMessageBox.critical(self, "Ошибка", f"Не удалось загрузить файл:\n{e}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())