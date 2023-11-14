module Board;

import Column;

Board::Board(uint16_t size)
	: m_matrix{size, std::vector<IColumn*>(size, nullptr)}
{

}

bool Board::ValidPlaceColumn(Position position) const
{
	if (std::find(m_columns.begin(), m_columns.end(), &position) != m_columns.end()) 
	{
		return false;
	}
	return true;
}

bool Board::ValidBridge(Position firstPosition, Position secondPosition) const
{
	int absValueX = abs(firstPosition.GetX() - secondPosition.GetX());
	int absValueY = abs(firstPosition.GetY() - secondPosition.GetY());
	if (absValueX == 0 || absValueY == 0) {
		return false;
	}
	if (absValueX + absValueY == 3) {
		return true;
	}
	return false;
}

Board::~Board() {
	for (int index = 0; index < m_columns.size(); index++) {
		delete m_columns[index];
	}
	for (int index1 = 0; index1 < m_matrix.size(); index1++) {
		for (int index2 = 0; index2 < m_matrix[index1].size(); index2++) {
			delete m_matrix[index1][index2];
		}
	}
}

IColumn* Board::GetElement(Position position) const
{
	return m_matrix[position.GetX()][position.GetY()];
}

void Board::PlaceColumn(Position& position, IPlayer* player)
{
	if (ValidPlaceColumn(position)) {
		IColumn* newColumn = new Column(player);
		m_matrix[position.GetX()][position.GetY()] = newColumn;
		m_columns.push_back(&position);
	}
}

void Board::MakeBridge(Position& firstPosition, Position& secondPosition, IPlayer* player)
{
	if (ValidBridge(firstPosition, secondPosition)) {
		Bridge *bridge = new Bridge(m_matrix[firstPosition.GetY()][firstPosition.GetX()],
			m_matrix[secondPosition.GetY()][secondPosition.GetX()]);
		m_bridges.push_back(bridge);
	}
}

void Board::RemoveBridge(Position& firstPosition, Position& secondPosition, IPlayer* player)
{
	if (!ValidBridge(firstPosition, secondPosition)) {
		//exception
		return;
	}

	Bridge* bridgeToRemove = new Bridge(m_matrix[firstPosition.GetY()][firstPosition.GetX()],
		m_matrix[secondPosition.GetY()][secondPosition.GetX()]);

	auto it = std::find(m_bridges.begin(), m_bridges.end(), bridgeToRemove);
	if (it != m_bridges.end()) {
		m_bridges.erase(it);
		delete bridgeToRemove;
	}
	else {
		// Bridge not found 

	}
}

Board::Board(const Board& otherBoard) {
	for (Position* position : otherBoard.m_columns) {
		this->m_columns.push_back(position);
	}
	for (const auto& bridgePair : otherBoard.m_bridges) {
		this->	m_bridges.push_back(bridgePair);
	}

	for (const std::vector<IColumn*>& row : otherBoard.m_matrix) {
		std::vector<IColumn*> newRow;
		for (IColumn* column : row) {
			newRow.push_back(column);
		}
		this->m_matrix.push_back(newRow);
	}
}