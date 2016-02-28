#include "RawLibrary.h"
#include <FileSystem.h>

QStringList RawLibrary::files() const
{
	QStringList retval;
	QString storage = storageSuffix();
	if (storage.contains("${arch}"))
	{
		QString cooked_storage = storage;
		cooked_storage.replace("${arch}", "32");
		retval.append(cooked_storage);
		cooked_storage = storage;
		cooked_storage.replace("${arch}", "64");
		retval.append(cooked_storage);
	}
	else
		retval.append(storage);
	return retval;
}

bool RawLibrary::filesExist(const QDir &base) const
{
	auto libFiles = files();
	for(auto file: libFiles)
	{
		QFileInfo info(base, file);
		qWarning() << info.absoluteFilePath() << "doesn't exist";
		if (!info.exists())
			return false;
	}
	return true;
}

QString RawLibrary::url() const
{
	if (!m_absolute_url.isEmpty())
	{
		return m_absolute_url;
	}

	if (m_base_url.isEmpty())
	{
		return QString("https://" + URLConstants::LIBRARY_BASE) + storageSuffix();
	}

	if(m_base_url.endsWith('/'))
	{
		return m_base_url + storageSuffix();
	}
	else
	{
		return m_base_url + QChar('/') + storageSuffix();
	}
}

bool RawLibrary::isActive() const
{
	bool result = true;
	if (m_rules.empty())
	{
		result = true;
	}
	else
	{
		RuleAction ruleResult = Disallow;
		for (auto rule : m_rules)
		{
			RuleAction temp = rule->apply(this);
			if (temp != Defer)
				ruleResult = temp;
		}
		result = result && (ruleResult == Allow);
	}
	if (isNative())
	{
		result = result && m_native_classifiers.contains(currentSystem);
	}
	return result;
}

void RawLibrary::setStoragePrefix(QString prefix)
{
	m_storagePrefix = prefix;
}

QString RawLibrary::defaultStoragePrefix()
{
	return "libraries/";
}

QString RawLibrary::storagePrefix() const
{
	if(m_storagePrefix.isEmpty())
	{
		return defaultStoragePrefix();
	}
	return m_storagePrefix;
}

QString RawLibrary::storageSuffix() const
{
	// non-native? use only the gradle specifier
	if (!isNative())
	{
		return m_name.toPath();
	}

	// otherwise native, override classifiers. Mojang HACK!
	GradleSpecifier nativeSpec = m_name;
	if (m_native_classifiers.contains(currentSystem))
	{
		nativeSpec.setClassifier(m_native_classifiers[currentSystem]);
	}
	else
	{
		nativeSpec.setClassifier("INVALID");
	}
	return nativeSpec.toPath();
}

QString RawLibrary::storagePath() const
{
	return FS::PathCombine(storagePrefix(), storageSuffix());
}

bool RawLibrary::storagePathIsDefault() const
{
	return m_storagePrefix.isEmpty();
}
