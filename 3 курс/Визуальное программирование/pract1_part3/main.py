import sys
from PySide6.QtCore import QUrl
from PySide6.QtGui import QGuiApplication
from PySide6.QtQuick import QQuickView

def main():
    app = QGuiApplication(sys.argv)

    view = QQuickView()
    view.setTitle("Инвентарь")
    view.setSource(QUrl.fromLocalFile("Inventory.qml"))
    view.setResizeMode(QQuickView.ResizeMode.SizeRootObjectToView)
    view.show()

    sys.exit(app.exec())

if __name__ == "__main__":
    main()