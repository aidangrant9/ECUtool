#include <QAbstractListModel>
#include "../core/DiagnosticSession.hpp"

class CommandView : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit CommandView(std::shared_ptr<DiagnosticSession> diagnosticSession, QWidget *parent = nullptr);

private:
	std::shared_ptr diagnosticSession{};
};