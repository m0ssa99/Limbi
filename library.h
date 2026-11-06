#pragma once
#include <QtCore>
#include <QtNetwork>
#include <functional>
#include <memory>

namespace Library {
constexpr qint64 MaxBytes = 8 * 1024 * 1024;
inline bool validate(const QByteArray &bytes, QJsonArray &entries, QString &error) {
    if(bytes.size()>MaxBytes) { error="Biblioteca este prea mare."; return false; }
    QJsonParseError parse; auto doc=QJsonDocument::fromJson(bytes,&parse);
    if(parse.error!=QJsonParseError::NoError || !doc.isArray() || doc.array().isEmpty()) { error="Biblioteca trebuie să fie o listă JSON nevidă."; return false; }
    QSet<int> ids;
    for(const auto &v:doc.array()) {
        auto e=v.toObject(); double value=e["id"].toDouble(-1);
        if(value<1 || value>2147483647 || value!=int(value) || ids.contains(int(value))) { error="ID invalid sau duplicat."; return false; }
        ids.insert(int(value));
        for(auto field:{"en","ro","it","meaning","exampleEn","exampleRo","exampleIt","kind"}) {
            if(!e[field].isString() || e[field].toString().trimmed().isEmpty() || e[field].toString().size()>5000) { error="Câmp lipsă sau invalid: "+QString(field); return false; }
        }
    }
    entries=doc.array(); return true;
}
inline bool manifest(const QByteArray &bytes, const QUrl &base, QUrl &url, QByteArray &hash, QString &error) {
    QJsonParseError parse; auto doc=QJsonDocument::fromJson(bytes,&parse); auto obj=doc.object();
    if(parse.error!=QJsonParseError::NoError || obj["schemaVersion"].toInt()!=1 || obj["version"].toString().isEmpty()) { error="Manifest incompatibil sau invalid."; return false; }
    hash=obj["sha256"].toString().toLatin1().toLower();
    if(!QRegularExpression("^[a-f0-9]{64}$").match(QString::fromLatin1(hash)).hasMatch()) { error="SHA-256 invalid."; return false; }
    url=base.resolved(QUrl(obj["url"].toString()));
    if(obj["url"].toString().isEmpty() || !url.isValid() || url.scheme()!="https" || url.host()!=base.host() || !url.userInfo().isEmpty()) { error="Pachetul trebuie să folosească HTTPS pe aceeași gazdă ca manifestul."; return false; }
    return true;
}
}

class LibraryUpdater : public QObject {
    QNetworkAccessManager network;
    bool busy=false;
    QString cachePath;
    void fetch(QUrl url, qint64 limit, std::function<void(QByteArray,QString)> done) {
        QNetworkRequest request(url); request.setTransferTimeout(20000);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::SameOriginRedirectPolicy);
        request.setRawHeader("User-Agent","LimbiQt/0.3");
        auto reply=network.get(request); auto data=std::make_shared<QByteArray>(); auto tooLarge=std::make_shared<bool>(false);
        connect(reply,&QNetworkReply::readyRead,this,[reply,data,tooLarge,limit]{
            data->append(reply->readAll()); if(data->size()>limit){*tooLarge=true;reply->abort();}
        });
        connect(reply,&QNetworkReply::finished,this,[reply,data,tooLarge,limit,done]{
            data->append(reply->readAll()); QString error;
            if(*tooLarge || data->size()>limit)error="Fișierul depășește limita admisă.";
            else if(reply->error()!=QNetworkReply::NoError)error=reply->errorString();
            else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()!=200)error="Răspuns HTTP neașteptat.";
            reply->deleteLater();done(*data,error);
        });
    }
public:
    explicit LibraryUpdater(QString path,QObject *parent=nullptr):QObject(parent),cachePath(path){}
    void check(QUrl url,std::function<void(QJsonArray,QString)> done) {
        if(busy){done({},"O verificare este deja în curs.");return;}
        if(!url.isValid() || url.scheme()!="https" || url.host().isEmpty() || !url.userInfo().isEmpty()){done({},"Introdu o adresă HTTPS validă pentru manifest.");return;}
        busy=true;
        fetch(url,65536,[this,url,done](QByteArray bytes,QString error){
            QUrl package; QByteArray hash;
            if(!error.isEmpty() || !Library::manifest(bytes,url,package,hash,error)){busy=false;done({},error);return;}
            QFile cached(cachePath);
            if(cached.open(QIODevice::ReadOnly)){
                auto local=cached.read(Library::MaxBytes+1); QJsonArray validated; QString localError;
                if(QCryptographicHash::hash(local,QCryptographicHash::Sha256).toHex()==hash && Library::validate(local,validated,localError)){busy=false;done({},"Biblioteca este deja la zi.");return;}
            }
            fetch(package,Library::MaxBytes,[this,done,hash](QByteArray data,QString error){
                QJsonArray entries;
                if(error.isEmpty() && QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex()!=hash) error="Verificarea SHA-256 a eșuat.";
                if(error.isEmpty())Library::validate(data,entries,error);
                if(error.isEmpty()){
                    QSaveFile file(cachePath);
                    if(!file.open(QIODevice::WriteOnly) || file.write(data)!=data.size() || !file.commit())error="Biblioteca nu a putut fi salvată. Versiunea anterioară este păstrată.";
                }
                busy=false; done(error.isEmpty()?entries:QJsonArray(),error.isEmpty()?QString("Biblioteca a fost actualizată."):error);
            });
        });
    }
};
