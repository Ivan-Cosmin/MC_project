#include "GameScreen.h"
#include "FileUtils.h"

#include "IGameSettings.h"


GameScreen::GameScreen(IGamePtr game, QWidget* parent)
	: QWidget{parent}
	, m_game{game}
	, m_layout{new QGridLayout{this}}
	, m_history{new HistoryWidget{this}}
	, m_firstPlayerBar{new PlayerBar{*game->GetFirstPlayer(), this}}
	, m_secondPlayerBar{new PlayerBar{*game->GetSecondPlayer(), this}}
	, m_board{new BoardWidget{*game->GetBoard(), game->GetTurn(),
			  game->GetFirstPlayer()->GetColor(), game->GetSecondPlayer()->GetColor(),
			  this}}
{
	m_layout->addWidget(m_firstPlayerBar, 0, 0, 1, 4);
	m_layout->addWidget(m_board, 1, 0, 1, 3);
	m_layout->addWidget(m_history, 1, 3);
	m_layout->addWidget(m_secondPlayerBar, 2, 0, 1, 4);
	setLayout(m_layout);
	m_history->setMaximumWidth(350);

	connect(m_board,
		SIGNAL(BoardClicked(const Position&, const Qt::MouseButton&)),
		SLOT(OnBoardClicked(const Position&, const Qt::MouseButton&)));

	connect(m_history,
		SIGNAL(itemActivated(QListWidgetItem*)),
		SLOT(OnHistoryClicked(QListWidgetItem*)));

	QString stylesheet{ FileUtils::StylesheetFileToString("./stylesheets/game.qss") };
	setStyleSheet(stylesheet);
}

void GameScreen::SetWindowFullScreen(bool isFullScreen)
{
	m_board->SetWindowFullScreen(isFullScreen);
}

void GameScreen::OnColumnPlaced(Position& position, IPlayer* player)
{
	m_history->AddColumnItem(player, position);
}

void GameScreen::OnBridgePlaced(Position& firstPos, Position& secondPos, IPlayer* player)
{
	m_history->AddBridgeItem(player, firstPos, secondPos, false);
}

void GameScreen::OnBridgeRemoved(Position& firstPos, Position& secondPos, IPlayer* player)
{
	m_history->AddBridgeItem(player, firstPos, secondPos, true);
}

void GameScreen::OnHistoryClicked(QListWidgetItem* item)
{
	qDebug() << m_history->currentRow();
	m_game->PreviewTable(m_history->currentRow());
	m_board->update();
}

void GameScreen::OnBoardClicked(const Position& position, const Qt::MouseButton& button)
{

	if (button == Qt::LeftButton)
	{
		m_game->PlaceColumn(position);
	}
	else if (button == Qt::RightButton)
	{

		auto currentColumn{ m_game->GetBoard()->GetElement(position) };
		bool currentPlayerTurn = currentColumn && currentColumn->GetPlayer() == m_game->GetTurn();

		if (!currentColumn) return;
		else if (m_selectedColumnPos == Position::EmptyPosition())
		{
			if (!currentPlayerTurn) return;
			m_selectedColumnPos = position;
			m_board->ChangeSelectedColumn(position);
		}
		else
		{
			if (m_selectedColumnPos != position && currentPlayerTurn)
			{
				if (m_game->GetBoard()->BridgeExists(m_selectedColumnPos, position))
				{
					m_game->RemoveBridge(m_selectedColumnPos, position);
				}
				else
				{
					m_game->MakeBridge(m_selectedColumnPos, position);
				}
			}

			m_selectedColumnPos = Position::EmptyPosition();
			m_board->ChangeSelectedColumn(Position::EmptyPosition());
		}
	}
}
