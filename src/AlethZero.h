/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file AlethZero.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#ifdef Q_MOC_RUN
#define BOOST_MPL_IF_HPP_INCLUDED
#endif

#include <map>
#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QAbstractListModel>
#include <QtCore/QMutex>
#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <libdevcore/RLP.h>
#include <libethcore/Common.h>
#include <libethcore/KeyManager.h>
#include <libethereum/State.h>
#include <libethereum/Executive.h>
#include <libwebthree/WebThree.h>
#include <libsolidity/CompilerStack.h>
#include "NatspecHandler.h"
#include "Common.h"
#include "Connect.h"
#include "Aleth.h"
#include "ZeroFace.h"
#include "Plugin.h"

class QListWidgetItem;
class QActionGroup;

namespace Ui {
class Main;
}

namespace dev
{

class SafeHttpServer;

namespace eth
{
class Client;
class State;
}

namespace aleth
{
namespace zero
{

class WebThreeServer;

class AlethZero: public ZeroFace
{
	Q_OBJECT

public:
	AlethZero();
	~AlethZero();

	WebThreeServer* web3Server() const override { return m_server.get(); }
	dev::SafeHttpServer* web3ServerConnector() const override { return m_httpConnector.get(); }

	AlethFace const* aleth() const { return &m_aleth; }
	AlethFace* aleth() { return &m_aleth; }

public slots:
	void note(QString _entry);
	void debug(QString _entry);
	void warn(QString _entry);
	QString contents(QString _file);

	void onKeysChanged();

private slots:
	// Application
	void on_about_triggered();
	void on_quit_triggered() { close(); }

	// Network
	void on_go_triggered();
	void on_net_triggered();
	void on_connect_triggered();
	void on_idealPeers_valueChanged(int);

	// Mining
	void on_mine_triggered();
	void on_prepNextDAG_triggered();

	// View
	void on_refresh_triggered();
	void on_preview_triggered();

	// Account management
	void on_killAccount_triggered();
	void on_reencryptKey_triggered();
	void on_reencryptAll_triggered();
	void on_exportKey_triggered();

	// Stuff concerning the blocks/transactions/accounts panels
	void on_ourAccounts_itemClicked(QListWidgetItem* _i);
	void on_ourAccounts_doubleClicked();

	// Special (debug) stuff
	void on_paranoia_triggered();
	void on_killBlockchain_triggered();
	void on_clearPending_triggered();
	void on_injectBlock_triggered();
	void on_forceMining_triggered();
	void on_usePrivate_triggered();
	void on_turboMining_triggered();
	void on_retryUnknown_triggered();
	void on_vmInterpreter_triggered();
	void on_vmJIT_triggered();
	void on_vmSmart_triggered();
	void on_rewindChain_triggered();
	void on_confirm_triggered();

	// Config
	void on_sentinel_triggered();

	void refreshAll();

signals:
	void poll();

private:
	template <class P> void loadPlugin() { Plugin* p = new P(this); initPlugin(p); }
	void initPlugin(Plugin* _p);
	void finalisePlugin(Plugin* _p);
	void unloadPlugin(std::string const& _name);

	p2p::NetworkPreferences netPrefs() const;

	void updateFee();
	void readSettings(bool _skipGeometry = false, bool _onlyGeometry = false);
	void writeSettings();

	void setPrivateChain(QString const& _private, bool _forceConfigure = false);

	void keysChanged();

	void installWatches();

	virtual void timerEvent(QTimerEvent*) override;

	void refreshNetwork();
	void refreshMining();
	void refreshBlockCount();
	void refreshBalances();

	void setBeneficiary(Address const& _b);
	std::string getPassword(std::string const& _title, std::string const& _for, std::string* _hint = nullptr, bool* _ok = nullptr);

	std::unique_ptr<Ui::Main> ui;
	std::unique_ptr<WebThreeDirect> m_webThree;	// TODO: move into Aleth.

	QByteArray m_networkConfig;	// TODO: move into Aleth.
	QStringList m_servers;	// TODO: move into Aleth.
	eth::KeyManager m_keyManager;	// TODO: move into Aleth.
	QString m_privateChain;	// TODO: move into Aleth.

	dev::Address m_beneficiary;	// TODO: move into Aleth?

	QActionGroup* m_vmSelectionGroup = nullptr;

	std::unique_ptr<dev::SafeHttpServer> m_httpConnector;	// TODO: move into Aleth.
	std::unique_ptr<WebThreeServer> m_server;	// TODO: move into Aleth.

	NatspecHandler m_natSpecDB;	// TODO: move into Aleth.

	class FullAleth: public Aleth
	{
	public:
		FullAleth(AlethZero* _az): m_az(_az) { init(); }

		WebThreeDirect* web3() const override { return m_az->m_webThree.get(); }
		eth::Client* ethereum() const override { return m_az->m_webThree->ethereum(); }
		std::shared_ptr<shh::WhisperHost> whisper() const override { return m_az->m_webThree->whisper(); }
		NatSpecFace* natSpec() override { return &m_az->m_natSpecDB; }

		Secret retrieveSecret(Address const& _address) const override;
		eth::KeyManager& keyManager() override { return m_az->m_keyManager; }
		void noteKeysChanged() override { m_az->refreshBalances(); }

		void noteSettingsChanged() override { m_az->writeSettings(); }

	private:
		AlethZero* m_az;
	};

	FullAleth m_aleth;

	Connect m_connect;
};

}
}
}