#include <QApplication>
#include <QObject>
#include <QThread>
#include <QDebug>

// GUI
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

//==============================================================================
// Utility Class for Sleep
//==============================================================================

namespace
{
	class DerivedFromQThread : protected QThread
	{
	public:
		static void msleep (unsigned long msecs) { QThread::msleep(msecs); }
	};
	void Sleep(unsigned long msecs) { DerivedFromQThread::msleep(msecs); }
}

//==============================================================================
// Work
//==============================================================================

class Work : public QObject
{
	Q_OBJECT
	Q_ENUMS(Type)

public:
	enum Type { INIT, START, STOP, QUIT };
	Work(Type type) : QObject(), m_type(type)
	{}

	void run()
	{
		switch (m_type)
		{
		case INIT:
			Sleep(1000 + rand() % 2000);
			break;

		case START:
			Sleep(1000 + rand() % 2000);
			break;

		case STOP:
			Sleep(1000 + rand() % 2000);
			break;

		case QUIT:
			Sleep(1000 + rand() % 2000);
			break;
		}

		qDebug() << "Work finished: " << m_type << QThread::currentThread();
	}

	Type GetType() const { return m_type; }

private:
	Type m_type;
};

Q_DECLARE_METATYPE(Work::Type)

//==============================================================================
// Controller
//==============================================================================

class Controller : public QObject
{
	Q_OBJECT

public:
	Controller() : QObject()
	{}

signals:
	void workFinished(Work::Type type);

public slots:
	void onWorkQueued(Work::Type type)
	{
		Work work(type);
		work.run();
		emit workFinished(work.GetType());
	}
};

//==============================================================================
// WorkDialog
//==============================================================================
class WorkDialog : public QWidget
{
	Q_OBJECT

public:
	WorkDialog() : QWidget()
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(m_startButton = new QPushButton("Start"));
		layout->addWidget(m_stopButton = new QPushButton("Stop"));
		layout->addWidget(m_quitButton = new QPushButton("Quit"));

		// Map Buttons with each Work types
		m_startButton->setProperty("workType", QVariant::fromValue(Work::START));
		m_stopButton->setProperty("workType", QVariant::fromValue(Work::STOP));
		m_quitButton->setProperty("workType", QVariant::fromValue(Work::QUIT));
		connect(m_startButton, SIGNAL(clicked()), this, SLOT(onWorkButtonClicked()));
		connect(m_stopButton, SIGNAL(clicked()), this, SLOT(onWorkButtonClicked()));
		connect(m_quitButton, SIGNAL(clicked()), this, SLOT(onWorkButtonClicked()));

		// Create WorkController thread
		m_workThread = new QThread(this);
		m_controller = new Controller();

		// Create connections for sending and recieving works
		m_controller->moveToThread(m_workThread);
		connect(this, SIGNAL(queueWork(Work::Type)), m_controller, SLOT(onWorkQueued(Work::Type)));
		connect(m_controller, SIGNAL(workFinished(Work::Type)), this, SLOT(onWorkFinished(Work::Type)));

		connect(m_workThread, SIGNAL(finished()), m_controller, SLOT(deleteLater()));

		startWork(Work::INIT);

		m_workThread->start();
	}

	~WorkDialog()
	{
		m_workThread->quit();
		m_workThread->wait();
	}


public slots:
	void onWorkButtonClicked()
	{
		if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
		{
			Work::Type workType = button->property("workType").value<Work::Type>();
			startWork(workType);
		}
	}

	void startWork(Work::Type type)
	{
		switch (type)
		{
		case Work::INIT:
		case Work::START:
		case Work::STOP:
			m_startButton->setEnabled(false);
			m_stopButton->setEnabled(false);
			break;

		case Work::QUIT:
			m_startButton->setEnabled(false);
			m_stopButton->setEnabled(false);
			m_quitButton->setEnabled(false);
			break;
		}

		emit queueWork(type);
	}

	void onWorkFinished(Work::Type type)
	{
		switch (type)
		{
		case Work::INIT:
			m_startButton->setEnabled(true);
			m_stopButton->setEnabled(false);
			break;

		case Work::START:
			m_startButton->setEnabled(false);
			m_stopButton->setEnabled(true);
			break;

		case Work::STOP:
			m_startButton->setEnabled(true);
			m_stopButton->setEnabled(false);
			break;

		case Work::QUIT:
			m_startButton->setEnabled(false);
			m_stopButton->setEnabled(false);
			m_quitButton->setEnabled(false);
			qApp->quit();
			break;
		}
	}

signals:
	void queueWork(Work::Type);

private:
	QThread* m_workThread;
	Controller* m_controller;

	QPushButton* m_startButton;
	QPushButton* m_stopButton;
	QPushButton* m_quitButton;
};

//==============================================================================
// Main
//==============================================================================

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	WorkDialog dialog;
	dialog.show();

	return app.exec();
}

//==============================================================================

#include "main.moc"
