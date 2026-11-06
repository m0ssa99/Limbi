#include "publisher.h"
#include <cstdlib>
static void require(bool result,const char *message){if(!result){qCritical()<<message;std::exit(1);}}
int main(int argc,char **argv){
    QCoreApplication app(argc,argv);QString error;QJsonArray output;
    QJsonObject e{{"id",1},{"en","hello"},{"ro","salut"},{"it","ciao"},{"meaning","salut"},{"exampleEn","Hello, Ana."},{"exampleRo","Salut, Ana."},{"exampleIt","Ciao, Ana."},{"kind","cuvânt"}};
    auto bytes=[&](QJsonArray a){return QJsonDocument(a).toJson();};
    require(Library::validate(bytes({e}),output,error),"valid entry rejected");
    require(!Library::validate(bytes({e,e}),output,error),"duplicate ID accepted");
    auto missing=e;missing.remove("exampleIt");require(!Library::validate(bytes({missing}),output,error),"missing translation accepted");
    auto fractional=e;fractional["id"]=1.5;require(!Library::validate(bytes({fractional}),output,error),"fractional ID accepted");
    require(!Library::validate("[]",output,error),"empty package accepted");
    require(!Library::validate("broken",output,error),"malformed JSON accepted");
    require(!Library::validate(QByteArray(Library::MaxBytes+1,'x'),output,error),"oversized content accepted");
    QJsonObject m{{"schemaVersion",1},{"version","test"},{"url","content.json"},{"sha256",QString(64,'a')}};
    QUrl url;QByteArray hash;QUrl base("https://raw.githubusercontent.com/user/repo/main/library/manifest.json");
    require(Library::manifest(QJsonDocument(m).toJson(),base,url,hash,error),"valid manifest rejected");
    require(url.path().endsWith("/library/content.json"),"relative URL resolved incorrectly");
    m["url"]="https://evil.example/content.json";require(!Library::manifest(QJsonDocument(m).toJson(),base,url,hash,error),"cross-host accepted");
    m["url"]="http://raw.githubusercontent.com/content.json";require(!Library::manifest(QJsonDocument(m).toJson(),base,url,hash,error),"HTTP accepted");
    m["url"]="content.json";m["sha256"]="bad";require(!Library::manifest(QJsonDocument(m).toJson(),base,url,hash,error),"invalid checksum accepted");
    QTemporaryDir temp; require(temp.isValid(),"temporary directory unavailable");
    QString source=temp.filePath("source.json"),destination=temp.filePath("library"),package,message;
    QFile input(source);require(input.open(QIODevice::WriteOnly),"cannot write fixture");input.write(bytes({e}));input.close();
    require(Publisher::prepare(source,destination,package,message),"publisher rejected valid source");
    QFile mf(destination+"/manifest.json");require(mf.open(QIODevice::ReadOnly),"manifest not generated");auto originalManifest=mf.readAll();mf.close();
    auto object=QJsonDocument::fromJson(originalManifest).object();QFile payload(destination+"/"+object["url"].toString());require(payload.open(QIODevice::ReadOnly),"content not generated");
    auto data=payload.readAll();payload.close();require(Library::validate(data,output,error),"published content unreadable by app");
    require(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex()==object["sha256"].toString().toLatin1(),"published checksum mismatch");
    require(input.open(QIODevice::WriteOnly|QIODevice::Truncate),"cannot replace fixture");input.write("broken JSON");input.close();
    require(!Publisher::prepare(source,destination,package,message),"publisher accepted broken input");
    require(mf.open(QIODevice::ReadOnly) && mf.readAll()==originalManifest,"invalid source changed existing manifest");
    qInfo()<<"Content, manifest and publisher checks passed";
}
