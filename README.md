# C Multiplayer Battleship

Questo repository contiene il codice sorgente e i makefile necessari per compilare ed eseguire un gioco multiplayer di Battleship in C. Il progetto include due componenti principali:

- **Server (Listener & Game)**: Da eseguire su un sistema Unix-like.
- **Client**: Si collega al server ed è progettato per essere eseguito su Windows.

## Prerequisiti

- Sistema Unix-like (Linux o macOS) per ospitare il server.
- Windows (con il Developer Command Line di Visual Studio) per compilare ed eseguire il client.

## Istruzioni per la Compilazione

### Server (Sistema Unix-like)

1. Apri un terminale sul tuo sistema Unix-like.
2. Vai alla directory contenente i file del progetto.
3. Esegui il seguente comando per compilare il server:

    ```make all```

    Questo comando creerà una directory `target` e compilerà due eseguibili:
    - `Listener`: il processo principale per l'hosting delle sessioni di gioco.
    - `Game`: il server di gioco che gestisce la logica di Battleship.

4. Dopo la compilazione, per avviare l'hosting delle partite, esegui:

    ```./target/Listener <NUMERO_DI_GIOCATORI_DI_CUI_FARE_HOSTING>```

    Sostituisci `<NUMERO_DI_GIOCATORI_DI_CUI_FARE_HOSTING>` con il numero di giocatore di cui si vuole effettivamente fare hosting. Attualmente sono supportate partite da 2 giocatori o da 4.

5. Per rimuovere i file eseguibili, esegui:

    ```make clean```

### Client (Sistema Windows)

1. Apri il Developer Command Line (DCL) di Visual Studio.
2. Vai alla directory del progetto.
3. Esegui il seguente comando per compilare il client usando `nmake`:

    ```nmake all```

    Questo comando compilerà l'eseguibile `Client` nella directory `target`.

4. Per collegarti a una partita ospitata, esegui:

    ```Client <INDIRIZZO_IP_HOST> <PORTA_SERVIZIO_HOST>```

    Sostituisci `<INDIRIZZO_IP_HOST>` e `<PORTA_SERVIZIO_HOST>` con l’indirizzo IP e la porta del server host. Quest'ultima sarà **2222** se si vuole fare una partita da 2 giocatori o **4444** se si vuole fare una partita da 4 giocatori.

5. Per rimuovere l'eseguibile del client, esegui:

    ```nmake clean```

## Note

- Assicurati che il server e il client siano connessi alla stessa rete per la funzionalità multiplayer.
- Il servizio client deve essere compilato sulla Developer Command Line di Visual Studio per garantire il corretto funzionamento del Makefile. Se non si ha a disposizione tale DCL, si consiglia di seguire questa guida: https://learn.microsoft.com/en-us/cpp/build/walkthrough-compile-a-c-program-on-the-command-line?view=msvc-170
