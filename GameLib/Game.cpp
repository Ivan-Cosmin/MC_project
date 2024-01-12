
#include <ranges>
#include <algorithm>

#include "Game.h"
#include "MinedGame.h"
#include "BulldozerGame.h"

IGamePtr IGame::Produce(const IGameSettings& settings)
{
	return std::make_shared<Game>(settings);
}

Game::Game(const IGameSettings& settings)
	: m_player1{ new Player(settings.GetFirstPlayerName(), settings.GetFirstPlayerColor(),
							settings.GetBridgeLimit(), settings.GetColumnLimit())}
	, m_player2{ new Player(settings.GetSecondPlayerName(), settings.GetSecondPlayerColor(),
							settings.GetBridgeLimit(), settings.GetColumnLimit())}
	, m_boardSize{ settings.GetTableSize() }
	, m_parser{ parser::ITwixtParser::Produce(settings.GetTableSize())}
	, m_gamemode{ EGamemode::MinedColumns }
{	
	m_board = std::make_shared<Board>(m_boardSize);
	m_turn = m_player1;
	m_minedGame = new MinedGame(m_board);

	if (m_gamemode == EGamemode::MinedColumns) m_minedGame->AddMines();
	if (m_board->GetSize() <= 6)
	{
		m_minimax = new Minimax(*m_board, m_boardSize - 1, m_player1, m_player2);
	}
}

Game::Game(const Game & other)
	: m_boardSize{other.m_boardSize}
{
	m_player1 = new Player(dynamic_cast<Player*>(other.m_player1));
	m_player2 = new Player(dynamic_cast<Player*>(other.m_player2));
	m_turn = (other.m_turn == other.m_player1) ? m_player1 : m_player2;
	m_board = std::make_shared<Board>(*other.m_board);
	m_minimax = new Minimax(*m_board, m_boardSize - 1, m_player1, m_player2);
}

Game::Game(Game && other) noexcept
	: m_boardSize{ other.m_boardSize }
 {
	m_player1 = new Player(dynamic_cast<Player*>(other.m_player1));
	m_player2 = new Player(dynamic_cast<Player*>(other.m_player2));
	m_turn = (other.m_turn == other.m_player1) ? m_player1 : m_player2;
	m_board = std::make_shared<Board>(*other.m_board);
	m_minimax = new Minimax(*m_board, m_boardSize - 1, m_player1, m_player2);

	other.m_board = nullptr;
	other.m_boardSize = 0;
	other.m_player1 = nullptr;
	other.m_player2 = nullptr;
	other.m_turn = nullptr;
	other.m_minimax = nullptr;
}

bool Game::PlaceColumn(Position position)
{
	bool isMinedColumn = false;

	if (dynamic_cast<const MinedColumn*>(m_board->GetElement(position)))
		isMinedColumn = true;

	bool isFirstPlayer = m_turn == m_player1;
	if ((position.GetRow() == 0 || position.GetRow() == m_board->GetSize() - 1) &&
		!isFirstPlayer) return false;

	if ((position.GetColumn() == 0 || position.GetColumn() == m_board->GetSize() - 1) &&
		isFirstPlayer) return false;

	if (isFirstPlayer)
	{if (!m_player1->HasColumnsToAdd()) return false;}
		else
	{if (!m_player2->HasColumnsToAdd()) return false;}

	if (!m_board->PlaceColumn(position, m_turn)) return false;

	NotifyPlaceColumn(position, m_turn);
	isFirstPlayer ? m_player1->DecreaseColumnNumber() : m_player2->DecreaseColumnNumber();
	m_parser->AddColumn(position.ToPair(), isFirstPlayer);
	ChangeTurn();

	if (m_gamemode == EGamemode::MinedColumns && isMinedColumn)
	{
		m_minedGame->DestroyArea(position, m_player1, m_player2);
		isFirstPlayer ? m_player1->SetDoubleTurn(true) : m_player2->SetDoubleTurn(true);
	}

	return true;

}

bool Game::MakeBridge(Position firstPos, Position secondPos)
{
	bool isFirstPlayer = m_turn == m_player1;
	if (isFirstPlayer)
	{if (!m_player1->HasBridgesToAdd()) return false;}
	else
	{if (!m_player2->HasBridgesToAdd()) return false;}

	if (!m_board->MakeBridge(firstPos, secondPos, m_turn)) return false;

	NotifyMakeBridge(firstPos, secondPos, m_turn);
	isFirstPlayer ? m_player1->DecreaseBridgeNumber():
					m_player2->DecreaseBridgeNumber();
	m_parser->AddBridge(false, firstPos.ToPair(), secondPos.ToPair());
	
	ComputePathToWin(0, firstPos, secondPos);

	return true;
}

bool Game::RemoveBridge(Position firstPos, Position secondPos)
{
	if (!m_board->RemoveBridge(firstPos, secondPos, m_turn)) return false;

	

	NotifyRemoveBridge(firstPos, secondPos, m_turn);
	m_parser->AddBridge(true, firstPos.ToPair(), secondPos.ToPair());
	m_turn == m_player1 ? m_player1->IncreaseBridgeNumber():
						  m_player2->IncreaseBridgeNumber();
	ComputePathToWin(1, firstPos, secondPos);

	return true;
}

void Game::SwapPlayers()
{
	EColor auxiliarColor = m_player1->GetColor();
	m_player1->SetColor(m_player2->GetColor());
	m_player2->SetColor(auxiliarColor);
	ChangeTurn();
}

IPlayer* Game::GetFirstPlayer() const
{
	return m_player1;
}

IPlayer* Game::GetSecondPlayer() const
{
	return m_player2;
}

void Game::PreviewTable(int historyIndex)
{
	//const auto& moveRepresentation = m_parser->GetHistoryMovesPositions(historyIndex);
	//if ()
	//m_board.reset();
	
}

Game& Game::operator=(const Game& rhs)
{
	if (this != &rhs) {
		
		delete m_player1;
		delete m_player2;

		m_boardSize = rhs.m_boardSize;
		m_player1 = new Player(dynamic_cast<Player*>(rhs.m_player1));
		m_player2 = new Player(dynamic_cast<Player*>(rhs.m_player2));
		m_turn = (rhs.m_turn == rhs.m_player1) ? m_player1 : m_player2;
		m_board = std::make_shared<Board>(*rhs.m_board);

	}
	return *this;
}

Game& Game::operator=(Game&& rhs) noexcept {
	if (this == &rhs)
	{
		return *this;
	}
		delete m_player1;
		delete m_player2;

		m_boardSize = rhs.m_boardSize;
		m_player1 = new Player(dynamic_cast<Player*>(rhs.m_player1));
		m_player2 = new Player(dynamic_cast<Player*>(rhs.m_player2));
		m_turn = (rhs.m_turn == rhs.m_player1) ? m_player1 : m_player2;
		m_board = std::make_shared<Board>(*rhs.m_board);

		rhs.m_board = nullptr;
		rhs.m_boardSize = 0;
	    rhs.m_player1 = nullptr;
	    rhs.m_player2 = nullptr;
	    rhs.m_turn = nullptr;

	return *this;
}

IPlayer* Game::GetTurn() const 
{
	return m_turn;
}

IBoardPtr Game::GetBoard() const 
{
	return m_board;
}

IPlayer* Game::CheckWinner() const
{
	if (m_turn == m_player1)
	{
		if (m_board->CheckWinner(0))
		{
			return m_player1;
		}
	}
	else
	{
		if (m_board->CheckWinner(1))
		{
			return m_player2;
		}
	}
	return nullptr;
}

void Game::AddObserver(ObserverPtr observer)
{
	m_observers.push_front(observer);
}

void Game::RemoveObserver(ObserverPtr observer)
{
	m_observers.remove_if(
		[observer](ObserverPtr weakObserver) {
			return weakObserver.expired() || weakObserver.lock() == observer.lock();
		});
}

void Game::NotifyPlaceColumn(Position position, IPlayer* player) const
{
	for (auto& observer : m_observers)
	{
		observer.lock()->OnColumnPlaced(position, player);
	}
}

void Game::NotifyMakeBridge(Position firstPos, Position secondPos, IPlayer* player) const
{
	for (auto& observer : m_observers)
	{
		observer.lock()->OnBridgePlaced(firstPos, secondPos, player);
	}
}

void Game::NotifyRemoveBridge(Position firstPos, Position secondPos, IPlayer* player) const
{
	for (auto& observer : m_observers)
	{
		observer.lock()->OnBridgeRemoved(firstPos, secondPos, player);
	}
}

void Game::ChangeTurn()
{
	if (auto player = dynamic_cast<Player*>(m_turn); player && player->GetDoubleTurn())
	{
		player->SetDoubleTurn(false);
		return;
	}

	m_turn = m_turn == m_player1 ? m_player2 : m_player1;
}

void Game::ComputePathToWin(bool action, Position& firstPos, Position& secondPos) const
{
	if (m_turn == m_player1)
	{
		m_board->ComputePathToWin(0, action, firstPos, secondPos);
	}
	else
	{
		m_board->ComputePathToWin(1, action, firstPos, secondPos);
	}
}

parser::STNGameRepresentation Game::GetSTNGameRepresentation() const
{
	parser::BoardRepresentation boardRepresentation{m_boardSize, std::vector<parser::Piece>{m_boardSize, parser::Piece::Empty}};
	for (int row = 0; row < m_boardSize; row++)
	{
		for (int column = 0; column < m_boardSize; column++)
		{
			auto currentColumn = m_board->GetElement(row, column);
			if (!currentColumn) continue;
			boardRepresentation[row][column] = currentColumn->GetPlayer() == m_player1
				? parser::Piece::FirstPlayer
				: parser::Piece::SecondPlayer;
		}
	}

	BridgeVector bridges = m_board->GetBridgesPositions();
	parser::MovesPositions movesRepresentation;
	for (auto bridge : bridges)
	{
		const auto& [firstPos, secondPos] = bridge;
		movesRepresentation.emplace_back(firstPos.ToPair(), secondPos.ToPair());
	}

	return std::make_pair(boardRepresentation, movesRepresentation);
}

Board Game::STNGameRepresentationToBoard(const parser::STNGameRepresentation& game) const
{
	const auto& [boardRepresentation, moves] = game;
	Board board(static_cast<uint16_t>(boardRepresentation.size()));
	for (uint16_t row = 0; row < boardRepresentation.size(); row++)
	{
		for (uint16_t column = 0; column < boardRepresentation.size(); column++)
		{
			if (boardRepresentation[row][column] == parser::Piece::Empty) continue;
			IPlayer* currentPlayer = boardRepresentation[row][column] == parser::Piece::FirstPlayer ? m_player1 : m_player2;
			board.PlaceColumn({ row, column }, currentPlayer);
		}
	}

	for (auto move : moves)
	{
		const auto& [firstPos, secondPos] = move;
		const auto& [firstPosRow, firstPosColumn] = firstPos;
		const auto& [secondPosRow, secondPosColumn] = secondPos;
		IPlayer* currentPlayer = board.GetElement(firstPosRow, firstPosColumn)->GetPlayer();

		board.MakeBridge({ firstPosRow, firstPosColumn }, { secondPosRow, secondPosColumn }, currentPlayer);
	}

	return std::move(board);
}

bool Game::SaveGame(const std::string_view path, StorageFormat format)
{
	switch (format)
	{
	case StorageFormat::STN:
		return parser::ITwixtParser::SaveSTN(GetSTNGameRepresentation(), path);
	case StorageFormat::PTG:
		return m_parser->SavePTG(path);
	}
	return false;
}

bool Game::LoadGame(const std::string_view path, StorageFormat format)
{
	//switch (format)
	//{
	//case StorageFormat::STN:
	//{
	//	auto representation = parser::ITwixtParser::LoadSTN(path);
	//	const auto& [board, moves] = representation;
	//	if (board.empty()) return false;

	//}
	//case StorageFormat::PTG:
	//{
	//	if (m_parser->LoadPTG(path))
	//	{

	//	}
	//}
	//}
	return false;
}

std::pair<BridgeVector, Position> Game::GetHint(int16_t depth, IPlayer* player) const
{
	return m_minimax->GetHint(depth, player);
}
