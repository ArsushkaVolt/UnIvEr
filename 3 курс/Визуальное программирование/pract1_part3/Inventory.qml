import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// Главный контейнер всего интерфейса
Rectangle {
    id: root
    width: 650
    height: 420
    color: "#23272a" // Тёмный фон приложения

    // Свойства состояния (property): хранят текущие экипированные предметы и выбранную ячейку
    property string equippedWeapon: "Нет"
    property string equippedArmor: "Нет"
    property int selectedIndex: -1 // -1 означает, что ничего не выбрано

    // Главный горизонтальный компоновщик (делыт окно на 3 колонки)
    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // =====================================================================
        // 1. КОЛОНКА: Сетка предметов инвентаря
        // =====================================================================
        ColumnLayout {
            spacing: 8

            Text {
                text: "Инвентарь"
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }

            // Двумерная сетка для ячеек
            GridLayout {
                columns: 4 // 4 колонки
                rowSpacing: 6
                columnSpacing: 6

                // Цикл для генерации 16 ячеек инвентаря
                Repeater {
                    model: 16

                    Rectangle {
                        width: 55
                        height: 55
                        // Изменяем цвет фона и рамки, если ячейка выделена мышью
                        color: selectedIndex === index ? "#4f545c" : "#2c2f33"
                        border.color: "#7289da"
                        border.width: selectedIndex === index ? 2 : 1
                        radius: 4

                        // Отображение иконки предмета внутри ячейки
                        Text {
                            anchors.centerIn: parent
                            text: index === 0 ? "⚔️" : (index === 1 ? "🛡️" : "")
                            font.pixelSize: 24
                        }

                        // Обработка клика по ячейке
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                selectedIndex = index // Запоминаем индекс выбранной ячейки

                                // Обновляем информацию о предмете и видимость кнопки «Экипировать»
                                if (index === 0) {
                                    itemName.text = "Меч"
                                    itemInfo.text = "Тип: Оружие\nУрон: +25\nПрочность: 100/100"
                                    equipBtn.visible = true
                                } else if (index === 1) {
                                    itemName.text = "Щит"
                                    itemInfo.text = "Тип: Броня\nЗащита: +15\nПрочность: 80/80"
                                    equipBtn.visible = true
                                } else {
                                    itemName.text = "Пусто"
                                    itemInfo.text = "Ячейка не содержит предметов."
                                    equipBtn.visible = false
                                }
                            }
                        }
                    }
                }
            }
        }

        // =====================================================================
        // 2. КОЛОНКА: Карточка выбранного предмета и кнопка экипировки
        // =====================================================================
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#2c2f33"
            radius: 4

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                // Название предмета
                Text {
                    id: itemName
                    text: "Выберите предмет"
                    color: "#7289da"
                    font.pixelSize: 15
                    font.bold: true
                }

                // Характеристики предмета
                Text {
                    id: itemInfo
                    text: "Кликните на предмет в инвентаре для просмотра свойств."
                    color: "#99aab5"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // Динамическая кнопка «Экипировать» (появляется только если в ячейке есть предмет)
                Button {
                    id: equipBtn
                    text: "Экипировать"
                    visible: false
                    Layout.topMargin: 10

                    // Логика переноса предмета в слот экипировки
                    onClicked: {
                        if (selectedIndex === 0) {
                            equippedWeapon = "Меч (+25 Урон)"
                        } else if (selectedIndex === 1) {
                            equippedArmor = "Щит (+15 Защита)"
                        }
                    }
                }

                Item { Layout.fillHeight: true } // Распорка для выравнивания по верхнему краю
            }
        }

        // =====================================================================
        // 3. КОЛОНКА: Слоты экипированного снаряжения персонажа
        // =====================================================================
        Rectangle {
            Layout.preferredWidth: 180
            Layout.fillHeight: true
            color: "#2c2f33"
            radius: 4

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Text {
                    text: "Снаряжение"
                    color: "#ffffff"
                    font.pixelSize: 15
                    font.bold: true
                }

                // Слот для Оружия
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "#23272a"
                    radius: 4
                    border.color: "#7289da"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        Text { text: "⚔️ Оружие:"; color: "#99aab5"; font.pixelSize: 10 }
                        // Значение автоматически обновляется при изменении свойства equippedWeapon
                        Text { text: equippedWeapon; color: "#ffffff"; font.pixelSize: 11; font.bold: true }
                    }
                }

                // Слот для Брони
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "#23272a"
                    radius: 4
                    border.color: "#7289da"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        Text { text: "🛡️ Броня:"; color: "#99aab5"; font.pixelSize: 10 }
                        // Значение автоматически обновляется при изменении свойства equippedArmor
                        Text { text: equippedArmor; color: "#ffffff"; font.pixelSize: 11; font.bold: true }
                    }
                }

                // Кнопка для сброса всех экипированных предметов
                Button {
                    text: "Снять всё"
                    Layout.fillWidth: true
                    onClicked: {
                        equippedWeapon = "Нет"
                        equippedArmor = "Нет"
                    }
                }

                Item { Layout.fillHeight: true } // Распорка
            }
        }
    }
}