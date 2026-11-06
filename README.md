# Limbi — prototip Windows 11

Un card în engleză, română și italiană, cu sens și exemple. Include 12 intrări demonstrative originale, fără conținut preluat din Oxford. Nu necesită internet, cont sau SDK .NET. Pronunția nu este inclusă în acest prototip.

## Pornire

Poți porni direct `Limbi.exe`. Este suficient să trimiți acest singur fișier colegului. Executabilul include scriptul și conținutul și le extrage în `%LOCALAPPDATA%\LimbiPrototype\app` la pornire. Folosește Windows PowerShell și .NET Framework din Windows; nu este încă o rescriere nativă și nu necesită SDK .NET. Executabilul nu este semnat digital, deci Windows poate cere confirmarea sursei. Pentru recompilare după modificări: `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build.ps1`.

Păstrează toate fișierele în același dosar și deschide `Start-Limbi.vbs` prin dublu click. Alternativ, din Windows PowerShell:

```powershell
powershell.exe -NoProfile -STA -ExecutionPolicy Bypass -File .\Limbi.ps1
```

Aplicația arată primul card. Închiderea ferestrei o trimite în tray. Din meniul pictogramei poți testa notificarea imediat, deschide cardul, vedea favoritele, configura programul sau închide complet aplicația. Windows poate ascunde pictograma sub săgeata de lângă ceas.

## Setări

Implicit: 09:00–20:00, interval aleatoriu de 45–75 minute. Dezactivează modul aleatoriu pentru interval fix. „Mai repetă” programează o notificare peste 10 minute, în interiorul programului activ. Pauza expiră în ziua următoare. După repaus se trimite cel mult o notificare, fără recuperarea celor ratate.

„Pornește cu Windows” creează un shortcut în Startup pentru utilizatorul curent. Păstrează dosarul în aceeași locație după activare. Debifează opțiunea înainte să muți sau ștergi aplicația. Fiecare coleg poate copia dosarul pe calculatorul său și activa separat pornirea automată.

Progresul este local în `%LOCALAPPDATA%\LimbiPrototype\state.json`. „Știu deja” scoate intrarea din selecție cât timp există intrări neînvățate; după parcurgerea tuturor, începe recapitularea. Nu există sincronizare între utilizatori.

## Limite și verificare manuală

Prototipul folosește notificări balloon din tray, nu notificări moderne cu istoric și butoane. Setările Windows „Nu deranja” pot suprima afișarea; cardul rămâne accesibil din tray. VBScript și PowerShell trebuie să fie permise de politicile calculatorului.

Verifică pe Windows 11: lansarea, click pe notificarea de test, salvarea unei expresii, repornirea și păstrarea progresului, închiderea în tray, setările și pornirea la următoarea autentificare. Interfața și livrarea notificărilor necesită verificare interactivă pe calculatorul utilizatorului.
