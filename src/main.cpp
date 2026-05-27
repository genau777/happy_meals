#include "mainwindow.h"
#include "clientapi.h"

#include <QApplication>
#include <QDebug>
#include <QString>

///
/// \brief Точка входа в приложение HappyMeals.
/// \param argc Количество аргументов командной строки.
/// \param argv Массив аргументов командной строки.
/// \return Код завершения приложения.
///
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    if (argc > 1) {
        quint16 port = (argc > 2) ? QString::fromLocal8Bit(argv[2]).toUShort() : 40000;
        ClientApi::getInstance()->connectToServer(QString::fromLocal8Bit(argv[1]), port);
    }

    // Глобальный стиль приложения
    a.setStyleSheet(
        "QMainWindow { background-color: #f8f9fa; }"
        "QWidget { font-family: 'Segoe UI', Roboto, sans-serif; font-size: 14px; color: #333; }"

        "QLineEdit { "
        "  border: 1px solid #ced4da; "
        "  border-radius: 12px; "
        "  padding: 12px 15px; "
        "  background-color: white; "
        "  color: #212529; "
        "}"
        "QLineEdit:focus { border: 2px solid #ff914d; }"

        "QPushButton { "
        "  background-color: #ff914d; "
        "  color: white; "
        "  border-radius: 12px; "
        "  padding: 10px 15px; "
        "  font-weight: bold; "
        "  text-transform: uppercase; "
        "  border: none; "
        "}"
        "QPushButton:hover { background-color: #ff7a2d; }"
        "QPushButton[text^='←'] { background: #e9ecef; color: #495057; border: 1px solid #ced4da; }"

        "QComboBox { "
        "  border: 1px solid #ced4da; "
        "  border-radius: 10px; "
        "  padding: 5px 10px; "
        "  background: white; "
        "  color: #333; "
        "}"
        "QComboBox::drop-down { border: none; }"
        "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #ff914d; margin-right: 10px; }"

        "QComboBox QAbstractItemView { "
        "  border: 1px solid #ced4da; "
        "  selection-background-color: #ff914d; "
        "  selection-color: white; "
        "  background-color: white; "
        "  color: #333; "
        "  outline: 0; "
        "}"

        "QListWidget { border: none; background: transparent; outline: none; }"
        "QListWidget::item { "
        "  background: white; "
        "  margin: 5px 10px; "
        "  padding: 15px; "
        "  border-radius: 15px; "
        "  border: 1px solid #e9ecef; "
        "  color: #333; "
        "}"
        "QListWidget::item:focus { border: 1px solid #ff914d; outline: none; }"
        "QListWidget::item:hover { border: 1px solid #ff914d; }"
        "QListWidget::item:selected { "
        "  background-color: #fff3e0; "
        "  color: #e66a20; "
        "  border: 2px solid #ff914d; "
        "  outline: none; "
        "}"
        );

    MainWindow w;
    w.setMinimumSize(430, 700);
    w.resize(450, 800);
    w.show();

    return a.exec();
}
