#include <QtWidgets>
#include <QRandomGenerator>
#include <QLockFile>
#include "library.h"

class Window : public QMainWindow {
    QSettings settings{"Limbi", "LimbiQt"};
    QJsonArray entries;
    QJsonObject current;
    QStringList saved, known, recent;
    QSystemTrayIcon *tray;
    QLabel *meta, *footer;
    QTextBrowser *card;
    QDateTime next;
    QDate paused;
    bool exiting = false;
    bool smoke = false;
    LibraryUpdater *updater=nullptr;
    QString libraryStatus="Biblioteca locală";
    QDateTime lastAttempt;
    QString manifestUrl() const { return settings.value("manifestUrl","https://raw.githubusercontent.com/m0ssa99/Limbi/main/library/manifest.json").toString(); }
    QString cachePath() const { return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/library.json"; }
    void checkLibrary(bool manual=false) {
        if(smoke)return;
        lastAttempt=QDateTime::currentDateTime();
        libraryStatus="Se verifică biblioteca…"; statusBar()->showMessage(libraryStatus);
        updater->check(QUrl(manifestUrl()),[this,manual](QJsonArray updated,QString message){
            if(!updated.isEmpty()) {
                // Păstrează intrările retrase pentru accesul la favoritele existente.
                QJsonArray archive, previous; QFile oldArchive(cachePath()+".saved");
                if(oldArchive.open(QIODevice::ReadOnly))previous=QJsonDocument::fromJson(oldArchive.readAll()).array();
                QSet<int> ids; for(auto v:updated)ids.insert(v.toObject()["id"].toInt());
                for(auto v:previous+entries) { int key=v.toObject()["id"].toInt(); if(saved.contains(QString::number(key)) && !ids.contains(key)){archive.append(v);ids.insert(key);} }
                QSaveFile backup(cachePath()+".saved"); if(backup.open(QIODevice::WriteOnly)){backup.write(QJsonDocument(archive).toJson());backup.commit();}
                entries=updated;
                bool found=false; for(auto v:entries)if(v.toObject()["id"]==current["id"]){current=v.toObject();found=true;break;}
                if(found)render();else pick();
            }
            libraryStatus=message; statusBar()->showMessage(message);
            if(manual)QMessageBox::information(this,"Bibliotecă",message);
        });
    }
    void schedule(int overrideMinutes = 0) {
        int minutes = settings.value("minutes",60).toInt();
        if (settings.value("random",true).toBool()) minutes += QRandomGenerator::global()->bounded(31)-15;
        next = QDateTime::currentDateTime().addSecs(60 * (overrideMinutes ? overrideMinutes : qMax(1,minutes)));
        updateFooter();
    }
    QString id() const { return QString::number(current["id"].toInt()); }
    void persist() { if(smoke)return; settings.setValue("saved",saved); settings.setValue("known",known); settings.setValue("recent",recent); settings.sync(); }
    void pick() {
        QList<QJsonObject> pool;
        for (const auto &value : entries) {
            auto e = value.toObject(); auto key = QString::number(e["id"].toInt());
            if (!known.contains(key) && !recent.contains(key)) pool.append(e);
        }
        if (pool.isEmpty()) for (const auto &value : entries) if (!known.contains(QString::number(value.toObject()["id"].toInt()))) pool.append(value.toObject());
        if (pool.isEmpty()) for (const auto &value : entries) pool.append(value.toObject());
        current = pool[QRandomGenerator::global()->bounded(pool.size())];
        recent.append(id()); while (recent.size()>5) recent.removeFirst();
        persist(); render();
    }
    void render() {
        QString html;
        auto section = [&](QString label, QString text, bool large) {
            html += "<p style='color:#67738c;font-size:11px;'>"+label+"</p><p style='color:#18243b;font-size:"+(large ? QString("22") : QString("15"))+"px;'>"+text.toHtmlEscaped()+"</p>";
        };
        section("ENGLISH",current["en"].toString(),true);
        section("ROMÂNĂ",current["ro"].toString(),true);
        section("ITALIANO",current["it"].toString(),true);
        section("CE ÎNSEAMNĂ",current["meaning"].toString(),false);
        section("ÎN CONTEXT",current["exampleEn"].toString(),false);
        html += "<p style='color:#52617b'>"+current["exampleRo"].toString().toHtmlEscaped()+"<br>"+current["exampleIt"].toString().toHtmlEscaped()+"</p>";
        card->setHtml(html); meta->setText(current["kind"].toString().toUpper()); updateFooter();
    }
    void updateFooter() {
        footer->setText(QString("%1 salvate  •  %2 învățate  •  %3").arg(saved.size()).arg(known.size()).arg(paused==QDate::currentDate() ? "Pauză pentru azi" : "Următoarea: "+next.toString("HH:mm")+" (în programul activ)"));
    }
    void reveal() { showNormal(); raise(); activateWindow(); }
    void notify() {
        pick();
        tray->showMessage("O expresie nouă te așteaptă", "Apasă pentru traducere, sens și un exemplu.",QSystemTrayIcon::Information,10000);
    }
    void preferences() {
        QDialog d(this); d.setWindowTitle("Setări"); QFormLayout layout(&d);
        QSpinBox minutes, start, end; minutes.setRange(15,240); start.setRange(0,23); end.setRange(1,24);
        minutes.setValue(settings.value("minutes",60).toInt()); start.setValue(settings.value("start",9).toInt()); end.setValue(settings.value("end",20).toInt());
        QCheckBox random("Variație aleatorie de ±15 minute"), startup("Pornește cu Windows"); random.setChecked(settings.value("random",true).toBool());
        QSettings run("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
        startup.setChecked(run.contains("LimbiQt"));
        layout.addRow("Interval (minute)",&minutes); layout.addRow("De la ora",&start); layout.addRow("Până la ora",&end); layout.addRow(&random); layout.addRow(&startup);
        QLabel info("Pornirea automată folosește locația actuală a aplicației.\nPăstrează dosarul aici după activare."); layout.addRow(&info);
        QDialogButtonBox buttons(QDialogButtonBox::Save|QDialogButtonBox::Cancel); layout.addRow(&buttons);
        QLineEdit source(manifestUrl()); source.setMinimumWidth(480); layout.insertRow(layout.rowCount()-1,"Manifest HTTPS",&source);
        QCheckBox automatic("Verifică zilnic biblioteca"); automatic.setChecked(settings.value("autoUpdate",true).toBool()); layout.insertRow(layout.rowCount()-1,&automatic);
        connect(&buttons,&QDialogButtonBox::rejected,&d,&QDialog::reject);
        connect(&buttons,&QDialogButtonBox::accepted,&d,[&]{
            QUrl url(source.text().trimmed());
            if(url.scheme()!="https" || url.host().isEmpty() || !url.userInfo().isEmpty()){QMessageBox::warning(&d,"Adresă invalidă","Introdu adresa HTTPS a fișierului manifest.json.");return;}
            if(start.value()>=end.value()) { QMessageBox::warning(&d,"Program invalid","Ora de început trebuie să fie înaintea orei de final."); return; }
            if(startup.isChecked()) run.setValue("LimbiQt","\""+QDir::toNativeSeparators(QCoreApplication::applicationFilePath())+"\" --tray"); else run.remove("LimbiQt");
            run.sync();
            if(run.status()!=QSettings::NoError) { QMessageBox::warning(&d,"Pornire automată","Windows nu a permis salvarea setării de pornire."); return; }
            settings.setValue("manifestUrl",url.toString()); settings.setValue("autoUpdate",automatic.isChecked());
            settings.setValue("minutes",minutes.value()); settings.setValue("start",start.value()); settings.setValue("end",end.value()); settings.setValue("random",random.isChecked()); schedule(); d.accept();
        }); d.exec();
    }
protected:
    void closeEvent(QCloseEvent *event) override {
        if (!exiting && !smoke && QSystemTrayIcon::isSystemTrayAvailable()) { hide(); event->ignore(); } else event->accept();
    }
public:
    Window(bool test=false) : smoke(test) {
        QFile file(":/content.json"); if(!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Content missing");
        entries=QJsonDocument::fromJson(file.readAll()).array(); if(entries.isEmpty()) throw std::runtime_error("Content empty");
        if(!smoke){QFile cache(cachePath());QString error;QJsonArray local;if(cache.open(QIODevice::ReadOnly) && Library::validate(cache.read(Library::MaxBytes+1),local,error))entries=local;}
        updater=new LibraryUpdater(cachePath(),this);
        saved=settings.value("saved").toStringList(); known=settings.value("known").toStringList(); recent=settings.value("recent").toStringList();
        setWindowTitle("Limbi • învață câte puțin"); resize(820,800); setMinimumSize(650,620);
        QPixmap icon(64,64); icon.fill(QColor("#405de6")); QPainter painter(&icon); painter.setPen(Qt::white); painter.setFont(QFont("Segoe UI",30,QFont::Bold)); painter.drawText(icon.rect(),Qt::AlignCenter,"L"); painter.end(); setWindowIcon(QIcon(icon));
        auto root=new QWidget; auto layout=new QVBoxLayout(root); layout->setContentsMargins(30,22,30,22); layout->setSpacing(14); setCentralWidget(root);
        auto top=new QHBoxLayout; auto title=new QLabel("Limbi"); title->setStyleSheet("font-size:30px;font-weight:700;color:#233455"); top->addWidget(title); top->addStretch();
        auto settingsButton=new QPushButton("Setări"); top->addWidget(settingsButton); connect(settingsButton,&QPushButton::clicked,this,[this]{preferences();}); layout->addLayout(top);
        auto updateButton=new QPushButton("Actualizează biblioteca"); top->insertWidget(2,updateButton); connect(updateButton,&QPushButton::clicked,this,[this]{checkLibrary(true);});
        auto subtitle=new QLabel("Un cuvânt. Trei limbi. Puțin mai mult în fiecare zi."); layout->addWidget(subtitle);
        meta=new QLabel; meta->setStyleSheet("color:#405de6;font-weight:600"); layout->addWidget(meta);
        card=new QTextBrowser; card->setFrameShape(QFrame::NoFrame); layout->addWidget(card,1);
        auto actions=new QHBoxLayout;
        auto button=[&](QString text,auto callback){auto b=new QPushButton(text); actions->addWidget(b); connect(b,&QPushButton::clicked,this,callback);};
        button("Salvează",[this]{if(!saved.contains(id()))saved.append(id());persist();render();});
        button("Știu deja",[this]{if(!known.contains(id()))known.append(id());persist();pick();});
        button("Mai repetă în 10 min",[this]{schedule(10);hide();});
        button("Următoarea →",[this]{pick();}); layout->addLayout(actions);
        footer=new QLabel; footer->setWordWrap(true); layout->addWidget(footer);
        setStyleSheet("QMainWindow,QDialog{background:#f4f6fc} QWidget{font-family:'Segoe UI';font-size:14px} QTextBrowser{background:white;border-radius:12px;padding:16px} QPushButton{background:white;border:1px solid #d5dced;border-radius:7px;padding:10px;color:#233455} QPushButton:hover{background:#e6ecff} QLabel{color:#60708b}");
        tray=new QSystemTrayIcon(windowIcon(),this); tray->setToolTip("Limbi — învață câte puțin"); auto menu=new QMenu(this);
        menu->addAction("Arată expresia",this,[this]{reveal();}); menu->addAction("Testează notificarea",this,[this]{notify();});
        menu->addAction("Expresii salvate",this,[this]{
            QDialog d(this); d.setWindowTitle("Expresii salvate"); d.resize(700,450); QVBoxLayout l(&d); QListWidget list; l.addWidget(&list);
            QJsonArray favorites=entries; QFile archive(cachePath()+".saved");if(archive.open(QIODevice::ReadOnly))favorites+=QJsonDocument::fromJson(archive.readAll()).array(); QSet<int> seen;
            for(const auto &v:favorites){ auto e=v.toObject(); int key=e["id"].toInt(); if(saved.contains(QString::number(key)) && !seen.contains(key)) { seen.insert(key);auto item=new QListWidgetItem(e["en"].toString()+" / "+e["ro"].toString()+" / "+e["it"].toString(),&list); item->setData(Qt::UserRole,e); } }
            if(!list.count())list.addItem("Încă nu ai expresii salvate.");
            connect(&list,&QListWidget::itemDoubleClicked,&d,[&](QListWidgetItem *item){auto e=item->data(Qt::UserRole).toJsonObject();if(!e.isEmpty()){current=e;render();d.accept();reveal();}});d.exec();
        });
        menu->addAction("Pauză / reia pentru azi",this,[this]{paused=paused==QDate::currentDate()?QDate():QDate::currentDate();schedule();});
        menu->addAction("Setări",this,[this]{preferences();});menu->addSeparator();menu->addAction("Închide aplicația",this,[this]{exiting=true;qApp->quit();});tray->setContextMenu(menu);
        connect(tray,&QSystemTrayIcon::messageClicked,this,[this]{reveal();}); connect(tray,&QSystemTrayIcon::activated,this,[this](auto reason){if(reason==QSystemTrayIcon::DoubleClick)reveal();});
        if(!smoke)tray->show(); pick();schedule();
        if(!smoke){auto updates=new QTimer(this);connect(updates,&QTimer::timeout,this,[this]{if(settings.value("autoUpdate",true).toBool() && (!lastAttempt.isValid() || lastAttempt.secsTo(QDateTime::currentDateTime())>=86400))checkLibrary();});updates->start(60000);if(settings.value("autoUpdate",true).toBool())QTimer::singleShot(1500,this,[this]{checkLibrary();});}
        auto timer=new QTimer(this); connect(timer,&QTimer::timeout,this,[this]{ auto now=QDateTime::currentDateTime(); int hour=now.time().hour(); if(paused!=now.date() && hour>=settings.value("start",9).toInt() && hour<settings.value("end",20).toInt() && now>=next){notify();schedule();} updateFooter();}); if(!smoke)timer->start(15000);
    }
};
int main(int argc,char **argv){
    QApplication app(argc,argv); app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("LimbiQt"); app.setOrganizationName("Limbi");
    const bool smoke=app.arguments().contains("--smoke-test");
    QString dir=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation); QDir().mkpath(dir);
    QLockFile lock(dir+"/instance.lock"); if(!smoke && !lock.tryLock(100)){QMessageBox::information(nullptr,"Limbi","Aplicația este deja pornită. Deschide-o din tray.");return 0;}
    try{Window window(smoke); if(!app.arguments().contains("--tray") || !QSystemTrayIcon::isSystemTrayAvailable())window.show(); if(smoke)QTimer::singleShot(250,&app,&QApplication::quit); return app.exec();}
    catch(const std::exception &error){qCritical()<<error.what();return 1;}
}
