#include <QtWidgets>
#include <QProcess>
#include "publisher.h"

class PublisherWindow : public QWidget {
    QLineEdit *source,*repository;
    QPlainTextEdit *log;
    QPushButton *generate,*publish;
    QProcess git;
    QString package;
    QStringList pending;
    QString repo;
    int step=0;
    bool running=false;
    void append(QString text){log->appendPlainText(text);}
    void finish(QString text){running=false;generate->setEnabled(true);publish->setEnabled(true);append(text);}
    void command(QStringList args){pending=args;append("git "+args.join(' '));git.setWorkingDirectory(repo);git.start("git",args);}
    bool prepare(){QString message;bool ok=Publisher::prepare(source->text(),QDir(repository->text()).filePath("library"),package,message);append(message);return ok;}
    void publishNow(){
        repo=QDir(repository->text()).absolutePath();
        if(!QFileInfo::exists(QDir(repo).filePath(".git"))){append("Alege rădăcina unui repository Git clonat local.");return;}
        running=true;generate->setEnabled(false);publish->setEnabled(false);step=0;command({"rev-parse","--show-toplevel"});
    }
    void completed(int code,QProcess::ExitStatus status){
        QString output=QString::fromUtf8(git.readAllStandardOutput()).trimmed();QString error=QString::fromUtf8(git.readAllStandardError()).trimmed();
        if(!output.isEmpty())append(output);if(!error.isEmpty())append(error);
        if(code!=0 || status!=QProcess::NormalExit){finish("Operația Git a eșuat. Nu s-a făcut reset sau force push. Verifică mesajul de mai sus; la eroare de push poți apăsa din nou Publică.");return;}
        switch(step++){
        case 0:
            if(QDir(output).canonicalPath()!=QDir(repo).canonicalPath()){finish("Selectează rădăcina repository-ului, nu un subdosar.");return;}
            command({"diff","--cached","--name-only"});break;
        case 1:
            if(!output.isEmpty()){finish("Există deja fișiere staged. Fă commit sau unstage pentru ele, apoi reîncearcă; utilitarul publică doar biblioteca.");return;}
            command({"branch","--show-current"});break;
        case 2:
            if(output!="main"){finish("Publicarea este configurată pentru ramura main. Schimbă ramura în Git înainte să publici.");return;}
            command({"remote","get-url","origin"});break;
        case 3:
            if(output!="https://github.com/m0ssa99/Limbi.git" && output!="https://github.com/m0ssa99/Limbi" && output!="git@github.com:m0ssa99/Limbi.git"){
                finish("Remote origin trebuie să fie repository-ul m0ssa99/Limbi.");return;
            }
            if(!prepare()){finish("Publicarea a fost oprită: sursa nu este validă.");return;}
            command({"add","--","library/"+package,"library/manifest.json"});break;
        case 4:command({"diff","--cached","--name-only"});break;
        case 5:
            if(output.isEmpty()){step=7;command({"push","origin","main"});}
            else command({"commit","-m","Update language content library","--","library/"+package,"library/manifest.json"});
            break;
        case 6:command({"push","origin","main"});break;
        default:finish("Publicare terminată. În aplicația Limbi apasă Actualizează biblioteca. GitHub poate avea o scurtă întârziere de cache.");break;
        }
    }
protected:
    void closeEvent(QCloseEvent *event) override {if(running){append("Așteaptă terminarea operației Git înainte de închidere.");event->ignore();}else event->accept();}
public:
    PublisherWindow(){
        setWindowTitle("Limbi Publisher — biblioteca GitHub");resize(850,650);
        auto layout=new QVBoxLayout(this);auto heading=new QLabel("Publică biblioteca de cuvinte și expresii");heading->setStyleSheet("font-size:23px;font-weight:600");layout->addWidget(heading);
        QSettings settings("Limbi","Publisher");
        QString defaultRepo=QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../..");
        if(!QFileInfo::exists(defaultRepo+"/.git"))defaultRepo=QDir::currentPath();
        repository=new QLineEdit(settings.value("repository",QDir(defaultRepo).absolutePath()).toString());
        source=new QLineEdit(settings.value("source",QDir(repository->text()).filePath("library-source.json")).toString());
        auto row=[&](QString label,QLineEdit *field,bool file){auto line=new QHBoxLayout;line->addWidget(new QLabel(label));line->addWidget(field,1);auto browse=new QPushButton("Alege…");line->addWidget(browse);layout->addLayout(line);connect(browse,&QPushButton::clicked,this,[=]{auto selected=file?QFileDialog::getOpenFileName(this,"Fișier sursă",field->text(),"JSON (*.json)"):QFileDialog::getExistingDirectory(this,"Repository local",field->text());if(!selected.isEmpty())field->setText(selected);});};
        row("Sursa JSON",source,true);row("Repository local",repository,false);
        auto info=new QLabel("Generează în <b>repository/library</b>. Sursa recomandată: <b>library-source.json</b>.<br>Publică face commit doar pentru pachet și manifest, apoi git push origin main.<br>Push trimite și eventualele commituri locale existente. Folosește autentificarea Git a calculatorului.<br>Intrările draft sunt permise pentru testare și își păstrează eticheta; nu sunt aprobate automat.");info->setWordWrap(true);layout->addWidget(info);
        auto actions=new QHBoxLayout;generate=new QPushButton("1. Generează JSON + manifest");publish=new QPushButton("2. Publică pe GitHub");actions->addWidget(generate);actions->addWidget(publish);layout->addLayout(actions);
        log=new QPlainTextEdit;log->setReadOnly(true);layout->addWidget(log,1);
        auto remember=[this]{QSettings s("Limbi","Publisher");s.setValue("source",source->text());s.setValue("repository",repository->text());};
        connect(generate,&QPushButton::clicked,this,[this,remember]{remember();prepare();});
        connect(publish,&QPushButton::clicked,this,[this,remember]{remember();publishNow();});
        git.setProcessChannelMode(QProcess::SeparateChannels);
        connect(&git,&QProcess::finished,this,&PublisherWindow::completed);
        connect(&git,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){if(error==QProcess::FailedToStart)finish("Git nu a putut porni. Instalează Git for Windows și autentifică-te pentru repository.");});
        setStyleSheet("QWidget{font-family:'Segoe UI';font-size:14px} QPushButton{padding:9px} QLineEdit{padding:6px}");
    }
};
int main(int argc,char **argv){
    QApplication app(argc,argv);
    auto args=app.arguments();
    if(args.contains("--generate")){
        int index=args.indexOf("--generate");if(args.size()<index+3)return 2;
        QString file,message;bool ok=Publisher::prepare(args[index+1],args[index+2],file,message);qInfo().noquote()<<message;return ok?0:1;
    }
    PublisherWindow window;window.show();if(args.contains("--smoke-test"))QTimer::singleShot(200,&app,&QApplication::quit);return app.exec();
}
