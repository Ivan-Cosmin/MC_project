#pragma once

#include <QStackedWidget>
#include <QMainWindow>

#include "MainMenuScreen.h"
#include "SettingsScreen.h"
#include "GameScreen.h"
#include "WidgetUtils.h"

#include "../GameLib/IGame.h"



class GameUI : public QMainWindow
{
    Q_OBJECT

public:
    GameUI(QWidget *parent = nullptr);
    ~GameUI();

public slots:
    void OnMainMenuButtonClicked(const EButtonPressed& button);

private:
    void InitializeMainMenu();
    void LoadFonts();

    QWidget* centralWidget;
    QStackedWidget* m_screens;
    MainMenuScreen* m_mainMenuScreen;
    SettingsScreen* m_settingsScreen;
    GameScreen* m_gameScreen;
};
