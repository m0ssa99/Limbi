#pragma once
#include "library.h"

namespace Publisher {
inline bool prepare(const QString &source,const QString &destination,QString &package,QString &message) {
    QFile input(source);
    if(!input.open(QIODevice::ReadOnly)){message="Nu pot citi fișierul sursă.";return false;}
    auto bytes=input.read(Library::MaxBytes+1); QJsonArray entries;
    if(!Library::validate(bytes,entries,message))return false;
    bytes=QJsonDocument(entries).toJson(QJsonDocument::Indented);
    if(bytes.size()>Library::MaxBytes){message="Setul formatat depășește 8 MiB.";return false;}
    QString hash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
    package="content-"+hash+".json";
    if(!QDir().mkpath(destination)){message="Nu pot crea dosarul de ieșire.";return false;}
    auto write=[&](QString name,QByteArray data){QSaveFile file(QDir(destination).filePath(name));return file.open(QIODevice::WriteOnly) && file.write(data)==data.size() && file.commit();};
    if(!write(package,bytes)){message="Nu pot salva pachetul.";return false;}
    QJsonObject manifest{{"schemaVersion",1},{"version",QDateTime::currentDateTimeUtc().toString("yyyy.MM.dd.HHmmss.zzz")},{"url",package},{"sha256",hash},{"count",entries.size()}};
    if(!write("manifest.json",QJsonDocument(manifest).toJson())){message="Nu pot salva manifestul. Manifestul anterior este păstrat.";return false;}
    int drafts=0;for(auto value:entries)if(value.toObject()["status"].toString()!="approved")++drafts;
    message=QString("Pachet creat: %1 intrări (%2 drafturi).\n%3").arg(entries.size()).arg(drafts).arg(QDir::toNativeSeparators(destination));
    return true;
}
}
