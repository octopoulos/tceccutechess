/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "engineconfiguration.h"
#include <QSet>
#include "engineoption.h"
#include "enginetextoption.h"
#include "engineoptionfactory.h"

EngineConfiguration::EngineConfiguration()
	: m_variants(QStringList() << "standard"),
	  m_whiteEvalPov(false),
	  m_pondering(false),
	  m_validateClaims(true),
	  m_restartMode(RestartAuto),
	  m_rating(0)
{
}

EngineConfiguration::EngineConfiguration(const QString& name,
                                         const QString& command,
					 const QString& protocol)
	: m_name(name),
	  m_command(command),
	  m_protocol(protocol),
	  m_variants(QStringList() << "standard"),
	  m_whiteEvalPov(false),
	  m_pondering(false),
	  m_validateClaims(true),
	  m_restartMode(RestartAuto),
	  m_rating(0)
{
}

EngineConfiguration::EngineConfiguration(const QVariant& variant)
	: m_variants(QStringList() << "standard"),
	  m_whiteEvalPov(false),
	  m_pondering(false),
	  m_validateClaims(true),
	  m_restartMode(RestartAuto),
	  m_rating(0)
{
	const QVariantMap map = variant.toMap();

	setName(map["name"].toString());
	setCommand(map["command"].toString());
	setWorkingDirectory(map["workingDirectory"].toString());
	setStderrFile(map["stderrFile"].toString());
	setProtocol(map["protocol"].toString());

	if (map.contains("initStrings"))
		setInitStrings(map["initStrings"].toStringList());
	if (map.contains("whitepov"))
		setWhiteEvalPov(map["whitepov"].toBool());
	if (map.contains("ponder"))
		setPondering(map["ponder"].toBool());

	if (map.contains("restart"))
	{
		const QString val(map["restart"].toString());
		if (val == "auto")
			setRestartMode(RestartAuto);
		else if (val == "on")
			setRestartMode(RestartOn);
		else if (val == "off")
			setRestartMode(RestartOff);
	}

	if (map.contains("validateClaims"))
		setClaimsValidated(map["validateClaims"].toBool());

	if (map.contains("variants"))
		setSupportedVariants(map["variants"].toStringList());

	if (map.contains("options"))
	{
		const QVariantList optionsList = map["options"].toList();
		EngineOption* option = nullptr;

		for (const QVariant& optionVariant : optionsList)
		{
			if ((option = EngineOptionFactory::create(optionVariant.toMap())) != nullptr)
				addOption(option);
		}
	}

	if (map.contains("rating"))
		setRating(map["rating"].toInt());
}

EngineConfiguration::EngineConfiguration(const EngineConfiguration& other)
	: m_name(other.m_name),
	  m_command(other.m_command),
	  m_workingDirectory(other.m_workingDirectory),
	  m_stderrFile(other.m_stderrFile),
	  m_protocol(other.m_protocol),
	  m_arguments(other.m_arguments),
	  m_initStrings(other.m_initStrings),
	  m_variants(other.m_variants),
	  m_whiteEvalPov(other.m_whiteEvalPov),
	  m_pondering(other.m_pondering),
	  m_validateClaims(other.m_validateClaims),
	  m_restartMode(other.m_restartMode),
	  m_rating(other.m_rating)
{
	const auto options = other.options();
	for (const EngineOption* option : options)
		addOption(option->copy());
}

EngineConfiguration& EngineConfiguration::operator=(EngineConfiguration&& other)
{
	// GV: Can this even happen? Leave it in anyway?
	if (this == &other)
		return *this;

	qDeleteAll(m_options);
	m_name = other.m_name;
	m_command = other.m_command;
	m_workingDirectory = other.m_workingDirectory;
	m_stderrFile = other.m_stderrFile;
	m_protocol = other.m_protocol;
	m_arguments = other.m_arguments;
	m_initStrings = other.m_initStrings;
	m_variants = other.m_variants;
	m_whiteEvalPov = other.m_whiteEvalPov;
	m_pondering = other.m_pondering;
	m_validateClaims = other.m_validateClaims;
	m_restartMode = other.m_restartMode;
	m_options = other.m_options;
	m_rating = other.m_rating;

	// other's destructor will cause a mess if its m_options isn't cleared
	other.m_options.clear();
	return *this;
}

EngineConfiguration::~EngineConfiguration()
{
	qDeleteAll(m_options);
}

QVariant EngineConfiguration::toVariant() const
{
	QVariantMap map;

	map.insert("name", m_name);
	map.insert("command", m_command);
	map.insert("workingDirectory", m_workingDirectory);
	map.insert("stderrFile", m_stderrFile);
	map.insert("protocol", m_protocol);

	if (!m_initStrings.isEmpty())
		map.insert("initStrings", m_initStrings);
	if (m_whiteEvalPov)
		map.insert("whitepov", true);
	if (m_pondering)
		map.insert("ponder", true);

	if (m_restartMode == RestartOn)
		map.insert("restart", "on");
	else if (m_restartMode == RestartOff)
		map.insert("restart", "off");

	if (!m_validateClaims)
		map.insert("validateClaims", false);

	if (m_variants.count("standard") != m_variants.count())
		map.insert("variants", m_variants);

	if (!m_options.isEmpty())
	{
		QVariantList optionsList;
		// TODO: use qAsConst() from Qt 5.7
		foreach (const EngineOption* option, m_options)
			optionsList.append(option->toVariant());

		map.insert("options", optionsList);
	}

	if (m_rating)
		map.insert("rating", m_rating);

	return map;
}

void EngineConfiguration::setName(const QString& name)
{
	m_name = name;
}

void EngineConfiguration::setCommand(const QString& command)
{
	m_command = command;
}

void EngineConfiguration::setProtocol(const QString& protocol)
{
	m_protocol = protocol;
}

void EngineConfiguration::setWorkingDirectory(const QString& workingDir)
{
	m_workingDirectory = workingDir;
}

void EngineConfiguration::setStderrFile(const QString& fileName)
{
	m_stderrFile = fileName;
}

void EngineConfiguration::setRating(const int rating)
{
	m_rating = rating > 0 ? rating : 0;
}

QString EngineConfiguration::name() const
{
	return m_name;
}

QString EngineConfiguration::command() const
{
	return m_command;
}

QString EngineConfiguration::workingDirectory() const
{
	return m_workingDirectory;
}

QString EngineConfiguration::stderrFile() const
{
	return m_stderrFile;
}

QString EngineConfiguration::protocol() const
{
	return m_protocol;
}

int EngineConfiguration::rating() const
{
	return m_rating;
}

QStringList EngineConfiguration::arguments() const
{
	return m_arguments;
}

void EngineConfiguration::setArguments(const QStringList& arguments)
{
	m_arguments = arguments;
}

void EngineConfiguration::addArgument(const QString& argument)
{
	m_arguments << argument;
}

QStringList EngineConfiguration::initStrings() const
{
	return m_initStrings;
}

void EngineConfiguration::setInitStrings(const QStringList& initStrings)
{
	m_initStrings = initStrings;
}

void EngineConfiguration::addInitString(const QString& initString)
{
	m_initStrings << initString.split('\n');
}

QStringList EngineConfiguration::supportedVariants() const
{
	return m_variants;
}

bool EngineConfiguration::supportsVariant(const QString& variant) const
{
	return m_variants.contains(variant);
}

void EngineConfiguration::setSupportedVariants(const QStringList& variants)
{
	m_variants = variants;
}

QList<EngineOption*> EngineConfiguration::options() const
{
	return m_options;
}

void EngineConfiguration::setOptions(const QList<EngineOption*>& options)
{
	qDeleteAll(m_options);
	m_options = options;
}

void EngineConfiguration::addOption(EngineOption* option)
{
	Q_ASSERT(option != nullptr);

	m_options << option;
}

void EngineConfiguration::setOption(const QString& name, const QVariant& value)
{
	// TODO: use qAsConst() from Qt 5.7
	foreach (EngineOption* option, m_options)
	{
		if (option->name() == name)
		{
			if (!option->isValid(value))
			{
				qWarning("Invalid value for engine option %s: %s",
					 qPrintable(name),
					 qPrintable(value.toString()));
			}
			else
				option->setValue(value);

			return;
		}
	}

	m_options << new EngineTextOption(name, value, value);
}

bool EngineConfiguration::whiteEvalPov() const
{
	return m_whiteEvalPov;
}

void EngineConfiguration::setWhiteEvalPov(bool whiteEvalPov)
{
	m_whiteEvalPov = whiteEvalPov;
}

bool EngineConfiguration::pondering() const
{
	return m_pondering;
}

void EngineConfiguration::setPondering(bool enabled)
{
	m_pondering = enabled;
}

EngineConfiguration::RestartMode EngineConfiguration::restartMode() const
{
	return m_restartMode;
}

void EngineConfiguration::setRestartMode(RestartMode mode)
{
	m_restartMode = mode;
}

bool EngineConfiguration::areClaimsValidated() const
{
	return m_validateClaims;
}

void EngineConfiguration::setClaimsValidated(bool validate)
{
	m_validateClaims = validate;
}

EngineConfiguration& EngineConfiguration::operator=(const EngineConfiguration& other)
{
	if (this != &other)
	{
		m_name = other.m_name;
		m_command = other.m_command;
		m_workingDirectory = other.m_workingDirectory;
		m_stderrFile = other.m_stderrFile;
		m_protocol = other.m_protocol;
		m_arguments = other.m_arguments;
		m_initStrings = other.m_initStrings;
		m_variants = other.m_variants;
		m_whiteEvalPov = other.m_whiteEvalPov;
		m_pondering = other.m_pondering;
		m_validateClaims = other.m_validateClaims;
		m_restartMode = other.m_restartMode;
		m_rating = other.m_rating;

		qDeleteAll(m_options);
		m_options.clear();

		// TODO: use qAsConst() from Qt 5.7
		foreach (const EngineOption* option, other.m_options)
			m_options.append(option->copy());
	}
	return *this;
}

bool EngineConfiguration::operator!=(const EngineConfiguration& other) const
{
	return !operator==(other);
}

static bool equivalent(const QStringList& l1, const QStringList& l2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
	for (auto str : l1)
		if (!l2.contains(str, cs))
			return false;
	for (auto str : l2)
		if (!l1.contains(str, cs))
			return false;
	return true;
}

bool EngineConfiguration::operator==(const EngineConfiguration& other) const
{
	if (m_whiteEvalPov != other.m_whiteEvalPov
		|| m_pondering != other.m_pondering
		|| m_validateClaims != other.m_validateClaims
		|| m_restartMode != other.m_restartMode
		|| m_rating != other.m_rating
		|| m_name != other.m_name
		|| m_command != other.m_command
		|| m_workingDirectory != other.m_workingDirectory
		|| m_stderrFile != other.m_stderrFile
		|| m_protocol != other.m_protocol
		|| m_arguments != other.m_arguments
		|| m_initStrings != other.m_initStrings
		|| !equivalent(m_variants, other.m_variants))
		return false;

	QSet<QString> names;
	for (auto op : m_options)
		names.insert(op->name());

	for (auto oop : other.m_options)
	{
		bool found = false;
		for (auto op : m_options)
		{
			if (oop->name() == op->name())
			{
				if (oop->value() == op->value())
					found = true;
				names.remove(op->name());
				break;
			}
		}
		if (!found)
			return false;
	}

	return names.isEmpty();
}
