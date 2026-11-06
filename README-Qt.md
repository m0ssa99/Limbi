# Limbi — C++ / Qt 6

Versiunea 0.3 este în `dist/LimbiQt-0.3/LimbiQt.exe`, iar pachetul pentru coleg este `dist/LimbiQt-0.3-Windows.zip`. Include actualizări de conținut prin HTTPS, cu adresă configurabilă. Vezi [CONTENT-GUIDE.md](CONTENT-GUIDE.md) pentru pregătire și publicare. Manifestul implicit indică repository-ul public `m0ssa99/Limbi`, ramura `main`; primul set trebuie publicat înainte de folosire. Codul nu este încă publicat automat pe GitHub.

Versiunea nouă are interfață Qt Widgets și nu folosește PowerShell la rulare.

## Utilizare

Dezarhivează `dist/LimbiQt-Windows.zip` și pornește `LimbiQt/LimbiQt.exe`. Păstrează bibliotecile și subdosarele lângă executabil. Pentru coleg trimite arhiva completă, nu doar executabilul. Nu este necesară instalarea Qt pe calculatorul lui.

Închiderea ferestrei păstrează aplicația în tray. Din meniul pictogramei poți testa notificarea, deschide favoritele (dublu click pentru cardul complet), pune pauză sau ieși complet. Setările includ interval fix/aleatoriu, ore active și pornire cu Windows. Pornirea automată folosește executabilul actual: dezactiveaz-o înainte să muți dosarul și reactiveaz-o după mutare.

Implicit: 45–75 minute, 09:00–20:00. „Mai repetă” programează notificarea peste 10 minute, în programul activ. După repaus se trimite cel mult o notificare la revenire. După marcarea tuturor intrărilor ca învățate, selecția reia conținutul pentru recapitulare.

Setările și progresul se salvează separat de prototipul vechi, în profilul Windows al fiecărui utilizator (QSettings: `HKEY_CURRENT_USER\Software\Limbi\LimbiQt`). Nu există sincronizare. Conținutul demonstrativ include aceleași 12 intrări originale. Pronunția nu este încă implementată. Notificările respectă restricțiile Windows și pot fi suprimate de „Nu deranja”. Executabilul nu este semnat digital.

## Compilare

Proiect CMake cu Qt 6 Widgets și C++17. Scriptul `Build-Qt.ps1` folosește Qt 6.12.0 și MinGW din `C:\Qt`, compilează Release, copiază bibliotecile cu windeployqt și produce arhiva în `dist`:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build-Qt.ps1
```

Validarea automată `LimbiQt.exe --smoke-test -platform offscreen` construiește fereastra și încarcă datele fără afișare. Clickul pe notificare, tray și pornirea la autentificare trebuie verificate interactiv pe Windows 11.

Qt este distribuit dinamic; condițiile și textele licenței instalate sunt în `C:\Qt\Licenses`. Acest pachet este un prototip privat, nu un instalator semnat.
