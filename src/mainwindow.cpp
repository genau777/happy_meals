#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "clientsessionmanager.h"
#include "clientapi.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QMessageBox>
#include <utility>
#include <QTextBrowser>

///
/// \brief Создает главное окно приложения HappyMeals.
///
/// Конструктор формирует основные экраны интерфейса:
/// авторизацию, регистрацию, главное меню, выбор ингредиентов,
/// фильтры поиска, результаты, избранное, историю и статистику.
///
/// На вход получает родительский виджет Qt.
/// На выходе создает интерфейс приложения и подключает обработчики действий пользователя.
///
/// \param parent Родительский виджет Qt.
///
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    while(ui->stackedWidget->count() > 1) {
        QWidget* w = ui->stackedWidget->widget(1);
        ui->stackedWidget->removeWidget(w);
        delete w;
    }

    // Sign In Page (Index 0)
    QWidget *loginPage = ui->stackedWidget->widget(0);
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
    loginLayout->setContentsMargins(40, 20, 40, 40);

    logEdit = new QLineEdit();
    logEdit->setPlaceholderText("Логин");
    logEdit->setMinimumHeight(45);
    passEdit = new QLineEdit();
    passEdit->setPlaceholderText("Пароль");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setMinimumHeight(45);
    btnEnter = new QPushButton("ВОЙТИ В СИСТЕМУ");
    btnEnter->setMinimumHeight(50);

    QPushButton *btnGoToRegister = new QPushButton("Нет аккаунта? Зарегистрироваться");
    btnGoToRegister->setMinimumHeight(40);
    btnGoToRegister->setStyleSheet("background-color: transparent; color: #6c757d; border: none;");

    loginLayout->addStretch(1);
    loginLayout->addWidget(new QLabel("🥘"), 0, Qt::AlignCenter);
    loginLayout->addWidget(new QLabel("<b>ШЕФ-ПОМОЩНИК</b>"), 0, Qt::AlignCenter);
    loginLayout->addWidget(new QLabel("<h3>ВХОД В СИСТЕМУ</h3>"), 0, Qt::AlignCenter);
    loginLayout->addWidget(logEdit);
    loginLayout->addWidget(passEdit);
    loginLayout->addWidget(btnEnter);
    loginLayout->addWidget(btnGoToRegister);
    loginLayout->addStretch(2);

    // Sign Up Page (Index 9)
    QWidget *registerPage = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerPage);
    registerLayout->setContentsMargins(40, 20, 40, 40);

    QLineEdit *regLogEdit = new QLineEdit();
    regLogEdit->setPlaceholderText("Логин");
    regLogEdit->setMinimumHeight(45);
    QLineEdit *regPassEdit = new QLineEdit();
    regPassEdit->setPlaceholderText("Пароль");
    regPassEdit->setEchoMode(QLineEdit::Password);
    regPassEdit->setMinimumHeight(45);
    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(45);
    btnRegister = new QPushButton("ЗАРЕГИСТРИРОВАТЬСЯ");
    btnRegister->setMinimumHeight(50);

    QPushButton *btnGoToLogin = new QPushButton("Уже есть аккаунт? Войти");
    btnGoToLogin->setMinimumHeight(40);
    btnGoToLogin->setStyleSheet("background-color: transparent; color: #6c757d; border: none;");

    registerLayout->addStretch(1);
    registerLayout->addWidget(new QLabel("🥘"), 0, Qt::AlignCenter);
    registerLayout->addWidget(new QLabel("<b>ШЕФ-ПОМОЩНИК</b>"), 0, Qt::AlignCenter);
    registerLayout->addWidget(new QLabel("<h3>РЕГИСТРАЦИЯ</h3>"), 0, Qt::AlignCenter);
    registerLayout->addWidget(regLogEdit);
    registerLayout->addWidget(regPassEdit);
    registerLayout->addWidget(emailEdit);
    registerLayout->addWidget(btnRegister);
    registerLayout->addWidget(btnGoToLogin);
    registerLayout->addStretch(2);

    QWidget *hubPage = new QWidget();
    QVBoxLayout *hubLayout = new QVBoxLayout(hubPage);
    hubLayout->setContentsMargins(50, 40, 50, 40);
    hubLayout->setSpacing(15);

    QLabel *welcomeLbl = new QLabel("<b>ДОБРО ПОЖАЛОВАТЬ, ШЕФ!</b>");
    welcomeLbl->setAlignment(Qt::AlignCenter);
    welcomeLbl->setStyleSheet("font-size: 20px; color: #ff914d;");

    btnStartSearch = new QPushButton("🔎 НАЧАТЬ ПОИСК РЕЦЕПТА");
    btnStartSearch->setMinimumHeight(60);

    btnGoToFavorites = new QPushButton("⭐ ИЗБРАННЫЕ РЕЦЕПТЫ");
    btnGoToFavorites->setMinimumHeight(55);

    btnShowHistory = new QPushButton("📜 ИСТОРИЯ ПОИСКА");
    btnShowHistory->setMinimumHeight(55);

    btnShowStats = new QPushButton("📊 МОЯ СТАТИСТИКА");
    btnShowStats->setMinimumHeight(55);

    QPushButton *btnLogout = new QPushButton("← ВЫЙТИ ИЗ АККАУНТА");
    btnLogout->setMinimumHeight(40);

    hubLayout->addStretch();
    hubLayout->addWidget(welcomeLbl);
    hubLayout->addWidget(btnStartSearch);
    hubLayout->addWidget(btnGoToFavorites);
    hubLayout->addWidget(btnShowHistory);
    hubLayout->addWidget(btnShowStats);
    hubLayout->addStretch();
    hubLayout->addWidget(btnLogout);
    ui->stackedWidget->addWidget(hubPage);

    QWidget *p2 = new QWidget();
    QVBoxLayout *l2 = new QVBoxLayout(p2);
    ingredientsList = new QListWidget();
    ingredientsList->setFocusPolicy(Qt::NoFocus);
    // Используем ингредиенты из базы данных (на английском, как в TheMealDB)
    ingredientsList->addItems({"Beef", "Chicken", "Pork", "Lamb", "Fish", "Salmon",
                               "Prawns", "Eggs", "Milk", "Cheese", "Butter",
                               "Tomatoes", "Onions", "Garlic", "Potatoes", "Carrots",
                               "Mushrooms", "Peppers", "Broccoli", "Spinach",
                               "Rice", "Pasta", "Flour", "Sugar", "Nuts"});
    ingredientsList->setSelectionMode(QAbstractItemView::MultiSelection);

    QPushButton *toStep2 = new QPushButton("ДАЛЕЕ ➔");
    QPushButton *backToHub = new QPushButton("← В ГЛАВНОЕ МЕНЮ");

    l2->addWidget(new QLabel("<b>ШАГ 1:</b> ЧТО ИСКЛЮЧИТЬ?"));
    l2->addWidget(ingredientsList);
    l2->addWidget(toStep2);
    l2->addWidget(backToHub);
    ui->stackedWidget->addWidget(p2);

    QWidget *p3 = new QWidget();
    QVBoxLayout *l3 = new QVBoxLayout(p3);
    cuisineBox = new QComboBox();
    cuisineBox->addItems({"Любая кухня", "Русская", "Итальянская", "Японская", "Китайская", "Мексиканская"});
    typeBox = new QComboBox();
    typeBox->addItems({"Любой тип", "Завтрак", "Второе", "Салат", "Десерт"});

    QPushButton *toStep3 = new QPushButton("ДАЛЕЕ ➔");
    QPushButton *backToStep1 = new QPushButton("← НАЗАД");

    l3->addStretch();
    l3->addWidget(new QLabel("<b>ШАГ 2:</b> КУХНЯ И ТИП"));
    l3->addWidget(cuisineBox);
    l3->addWidget(typeBox);
    l3->addWidget(toStep3);
    l3->addWidget(backToStep1);
    l3->addStretch();
    ui->stackedWidget->addWidget(p3);

    QWidget *p4 = new QWidget();
    QVBoxLayout *l4 = new QVBoxLayout(p4);
    timeSlider = new QSlider(Qt::Horizontal);
    timeSlider->setRange(10, 120);
    timeSlider->setValue(60);
    QLabel *tLbl = new QLabel("Макс. время: 60 мин");
    complexityBox = new QComboBox();
    complexityBox->addItems({"Любая сложность", "Легко", "Средне", "Сложно"});

    btnSearch = new QPushButton("🔍 НАЙТИ РЕЦЕПТ");
    btnSearch->setMinimumHeight(50);
    QPushButton *backToStep2 = new QPushButton("← НАЗАД");

    l4->addStretch();
    l4->addWidget(tLbl);
    l4->addWidget(timeSlider);
    l4->addWidget(complexityBox);
    l4->addWidget(btnSearch);
    l4->addWidget(backToStep2);
    l4->addStretch();
    ui->stackedWidget->addWidget(p4);

    QWidget *p5 = new QWidget();
    QVBoxLayout *l5 = new QVBoxLayout(p5);
    resultsList = new QListWidget();
    resultsList->setFocusPolicy(Qt::NoFocus);

    dishDescription = new QTextBrowser();
    dishDescription->setPlaceholderText("Выберите блюдо...");
    dishDescription->setStyleSheet("background: white; border-radius: 15px; padding: 10px; border: 1px solid #e9ecef;");

    btnFavorite = new QPushButton("⭐ В ИЗБРАННОЕ");
    btnFavorite->setEnabled(false);
    btnBack = new QPushButton("← К ФИЛЬТРАМ");

    l5->addWidget(new QLabel("<b>НАЙДЕННЫЕ РЕЦЕПТЫ:</b>"));
    l5->addWidget(resultsList, 1);
    l5->addWidget(new QLabel("<b>ПОДРОБНОЕ ОПИСАНИЕ:</b>"));
    l5->addWidget(dishDescription, 2);
    l5->addWidget(btnFavorite);
    l5->addWidget(btnBack);
    ui->stackedWidget->addWidget(p5);

    QWidget *p6 = new QWidget();
    QVBoxLayout *l6 = new QVBoxLayout(p6);
    favoritesList = new QListWidget();
    btnBackFromFav = new QPushButton("← В ГЛАВНОЕ МЕНЮ");
    l6->addWidget(new QLabel("<b>⭐ ВАШИ ИЗБРАННЫЕ РЕЦЕПТЫ:</b>"));
    l6->addWidget(favoritesList);
    l6->addWidget(btnBackFromFav);
    ui->stackedWidget->addWidget(p6);

    QWidget *p7 = new QWidget();
    QVBoxLayout *l7 = new QVBoxLayout(p7);
    historyList = new QListWidget();
    QPushButton *backFromHist = new QPushButton("← В ГЛАВНОЕ МЕНЮ");
    l7->addWidget(new QLabel("<b>📜 ИСТОРИЯ ПОИСКОВ:</b>"));
    l7->addWidget(historyList);
    l7->addWidget(backFromHist);
    ui->stackedWidget->addWidget(p7);

    QWidget *p8 = new QWidget();
    QVBoxLayout *l8 = new QVBoxLayout(p8);
    statsLabel = new QLabel();
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setStyleSheet("font-size: 16px; padding: 20px; background: white; border-radius: 15px;");
    QPushButton *backFromStats = new QPushButton("← В ГЛАВНОЕ МЕНЮ");
    l8->addStretch();
    l8->addWidget(new QLabel("<b>📊 ВАША СТАТИСТИКА:</b>"));
    l8->addWidget(statsLabel);
    l8->addStretch();
    l8->addWidget(backFromStats);
    ui->stackedWidget->addWidget(p8);

    // Add register page as index 9
    ui->stackedWidget->addWidget(registerPage);

    // Sign In page connections
    connect(btnEnter, &QPushButton::clicked, [this]() {
        ClientSessionManager::instance().requestLogin(logEdit->text(), passEdit->text());
    });
    connect(btnGoToRegister, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(9);
    });

    // Sign Up page connections
    connect(btnRegister, &QPushButton::clicked, [this, regLogEdit, regPassEdit]() {
        ClientSessionManager::instance().requestRegister(regLogEdit->text(), regPassEdit->text(), emailEdit->text());
    });
    connect(btnGoToLogin, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });

    // Auth result handlers
    connect(&ClientSessionManager::instance(), &ClientSessionManager::loginResult, [this](bool success, const QString &msg) {
        if (success) {
            ui->stackedWidget->setCurrentIndex(1);
        } else {
            qDebug() << "Login failed:" << msg;
            QMessageBox::warning(this, "Ошибка входа", msg);
        }
    });
    connect(&ClientSessionManager::instance(), &ClientSessionManager::registerResult, [this](bool success, const QString &msg) {
        if (success) {
            QMessageBox::information(this, "Регистрация", "Регистрация успешна. Теперь войдите в систему.");
            ui->stackedWidget->setCurrentIndex(0);
        } else {
            qDebug() << "Registration failed:" << msg;
            QMessageBox::warning(this, "Ошибка регистрации", msg);
        }
    });
    connect(btnLogout, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(0); });
    connect(btnStartSearch, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(2); });
    connect(btnGoToFavorites, &QPushButton::clicked, [this](){
        updateFavoritesList(); ui->stackedWidget->setCurrentIndex(6);
    });
    connect(btnShowHistory, &QPushButton::clicked, [this](){
        updateHistoryList(); ui->stackedWidget->setCurrentIndex(7);
    });
    connect(btnShowStats, &QPushButton::clicked, [this](){
        updateStatistics(); ui->stackedWidget->setCurrentIndex(8);
    });
    connect(toStep2, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(3); });
    connect(toStep3, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(4); });
    connect(backToHub, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(1); });
    connect(backToStep1, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(2); });
    connect(backToStep2, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(3); });
    connect(btnBack, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(4); });
    connect(btnBackFromFav, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(1); });
    connect(backFromHist, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(1); });
    connect(backFromStats, &QPushButton::clicked, [this](){ ui->stackedWidget->setCurrentIndex(1); });

    connect(timeSlider, &QSlider::valueChanged, [tLbl](int v){
        tLbl->setText(QString("Макс. время: %1 мин").arg(v));
    });

    connect(btnSearch, &QPushButton::clicked, this, [this]() {
        FilterCriteria cr;
        for (int i = 0; i < ingredientsList->count(); ++i)
            if (ingredientsList->item(i)->isSelected())
                cr.excludedIngredients << ingredientsList->item(i)->text();

        QString selectedCuisine = cuisineBox->currentText();
        cr.cuisine = (selectedCuisine.contains("Любая")) ? "" : selectedCuisine;

        QString selectedType = typeBox->currentText();
        cr.type = (selectedType.contains("Любой")) ? "" : selectedType;

        cr.maxTime = timeSlider->value();
        cr.maxComplexity = (complexityBox->currentIndex() == 0) ? 3 : complexityBox->currentIndex();

        QString historyEntry =
            QString("Исключить: [%1], кухня: %2, тип: %3, время до: %4 мин, сложность: %5")
                .arg(cr.excludedIngredients.join(", "))
                .arg(cr.cuisine.isEmpty() ? "Любая" : cr.cuisine)
                .arg(cr.type.isEmpty() ? "Любой" : cr.type)
                .arg(cr.maxTime)
                .arg(complexityBox->currentText());

        Q_UNUSED(historyEntry);

        QStringList foundTitles = ClientApi::getInstance()->findDishes(
            cr.excludedIngredients,
            cr.cuisine,
            cr.type,
            cr.maxTime,
            cr.maxComplexity
            );

        resultsList->clear();
        resultsList->clearSelection();
        dishDescription->clear();
        dishDescription->setPlaceholderText("Выберите блюдо...");
        btnFavorite->setText("⭐ ДОБАВИТЬ В ИЗБРАННОЕ");
        btnFavorite->setEnabled(false);

        if (foundTitles.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem("Рецепты не найдены.");
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            resultsList->addItem(item);
        } else {
            for (const QString& entry : foundTitles) {
                QStringList fields = entry.split('\t');
                QString title = fields.value(0).trimmed();
                QString time = fields.value(1).trimmed();
                QString label = time.isEmpty()
                                    ? title
                                    : QString("%1 — %2 мин").arg(title, time);

                QListWidgetItem *item = new QListWidgetItem("🍽️ " + label);
                item->setData(Qt::UserRole, title);
                resultsList->addItem(item);
            }
        }

        ui->stackedWidget->setCurrentIndex(5);
    });

    connect(resultsList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString title = item->data(Qt::UserRole).toString();
        if (title.isEmpty()) {
            return;
        }

        // Get detailed dish information from database
        QString html = ClientApi::getInstance()->getDishDetails(title);
        dishDescription->setHtml(html);

        bool isFav = ClientSessionManager::instance().isFavorite(title);
        btnFavorite->setEnabled(true);
        btnFavorite->setText(isFav ? "★ В ИЗБРАННОМ" : "⭐ ДОБАВИТЬ В ИЗБРАННОЕ");
    });

    connect(btnFavorite, &QPushButton::clicked, this, [this]() {
        if(QListWidgetItem *cur = resultsList->currentItem()) {
            QString title = cur->data(Qt::UserRole).toString();
            if (title.isEmpty()) title = cur->text();

            ClientSessionManager::instance().toggleFavorite(title);
            bool isFav = ClientSessionManager::instance().isFavorite(title);
            btnFavorite->setText(isFav ? "★ В ИЗБРАННОМ" : "⭐ ДОБАВИТЬ В ИЗБРАННОЕ");
            updateFavoritesList();
        }
    });

    ui->stackedWidget->setCurrentIndex(0);
}

///
/// \brief Обновляет список избранных рецептов.
///
/// Метод очищает текущий список избранного,
/// получает актуальный список из ClientSessionManager
/// и отображает его в интерфейсе.
///
void MainWindow::updateFavoritesList() {
    favoritesList->clear();
    QStringList favorites = ClientSessionManager::instance().getFavoritesList();
    if (favorites.isEmpty()) {
        favoritesList->addItem("У вас пока нет избранных рецептов");
    } else {
        for (const QString& fav : favorites) {
            QListWidgetItem *item = new QListWidgetItem("⭐ " + fav);
            favoritesList->addItem(item);
        }
    }
}

///
/// \brief Обновляет список истории поиска.
///
/// Метод очищает текущий список истории,
/// получает сохраненные поисковые запросы
/// и отображает их на соответствующем экране.
///
void MainWindow::updateHistoryList() {
    historyList->clear();
    auto hist = ClientSessionManager::instance().getSearchHistory();
    for(const auto& h : hist) {
        historyList->addItem(h);
    }
}

///
/// \brief Обновляет статистику пользователя.
///
/// Метод получает статистику текущего пользователя
/// и выводит ее в текстовую метку statsLabel.
///
void MainWindow::updateStatistics() {
    QString stats = ClientSessionManager::instance().getStatistics();
    if (stats.startsWith("ERROR")) {
        statsLabel->setText("❌ " + stats.mid(6)); // Remove "ERROR:" prefix
    } else if (stats.startsWith("OK:")) {
        statsLabel->setText("✅ " + stats.mid(3)); // Remove "OK:" prefix
    } else {
        statsLabel->setText(stats);
    }
}

///
/// \brief Уничтожает главное окно приложения.
///
/// Освобождает память, занятую объектом пользовательского интерфейса.
///
MainWindow::~MainWindow() { delete ui; }
