# Hvordan implementasjonen din fungerer?
    har testet d1_test_client og d2_test_clinet med oppgitte serverne (d1_server, d2_server, d1_dump) og de ga meg forvenet output.
    i starten da jeg skrev koden i bunnlaget i d1_udp.c, fikk jeg ikke til teste koden med den første versjonen av server som ble lagt ut.
    siden jeg jobbet i en windows OS med ubuntu 18.04 og testet koden med rehat server og da feilet flere ganger. så oppdaget at flere servere
    er lagt ut i uio sin github for linux, windows og mac. så sjekket igjen med windows serveren(ubuntu), men dessverre feilet igjen 
    og tror at det var pga min ubuntu(versjon 18.01). igjen testet koden med powershell, så feile igjen, så endte opp å teste koden med IFI maskinen 
    , så fikk jeg forventet output for d1_server og d1_dump.

    d1_udp.c består av flere funksjoner som fungerer slik:

        calculate_checksum:
            regner ut sjekksummen for å oppdage error i pakken.
            funksjonen itererer gjennom packet bytes, regner ut
            sjekksummen for partall og oddetall og kombinerer dem for den endelige sjekksummen.

        d1_create_client:
            funksjonen returnerer enten NULL eller en D1Peer struct.
            allokerer memmory for D1Peer STRUCT, så oppretter en UDP socket. 

        d1_delete:
            funksjonen lukker socket forbindelsen med en D1Peer struct om D1Peer pekeren ikke er NULL ,
            så frigjøre memoryen.

        d1_get_peer_info:
            funksjonen bestemmer om socket adressen hører til serveren med den oppgitte hostname og port.

        d1_recv_data:
            mottar en pakke, så sjekker size og sjekksummen, om det er ok, kopiere den i en buffer, 
            hvis size eller sjekksummen ikke stemmer, så sender en ACK.

        d1_wait_ack:
            venter på en ACk etter en pakke er sendt.
            denne funksjon er ment å bruke i d1_send_data. fungerer slik at den prøver en mekanisme
            for å motta en ack og den ack-en matcher med forventet seqno, så oppdatere seqno ellers prøver å sende pakken på nytt.

        d1_send_data:
            sender pakke ovet netwerk med en D1 header.
            konstruerer en pakke med en D1 HEADER, data bufferet og beregnet sjekksum,
            så sender pakken med UDP og venter for en ack ved å kalle på d1_wait_ack.


        d1_send_ack:
            sender en ack pakke til en peer.
            strukturer en Ack pakke med en gitt seqno og sender den til spesifisert peer.


    d2_lookup.c består av flere funksjoner som fungerer slik:

        d2_client_create:
            oppretter informasjonen for den oppgitte name og port og 
            lagrer dem in en D2client struct som er allocate, kaller på d1_create_client
            om å sjekke at den ikke er NULL, så returnere den.

        d2_client_delete:
            det er hjelpefunksjon for å slette en client   

        d2_send_request:
            sender en pakke request med den oppgitte iden.

        d2_recs_response_size:
            venter for en pakke fra server, returnerer antall noder som vil være med i flere pakkeresponser.


        d2_recv_response:
            mottar en respons fra serveren ved å kalle på d1_recv_data
            ####
                Jeg hadde utfordring her når jeg kjørte koden, 
                en gang fungerer for d1, så neste gang for d2,
                derfor fikk veiledning fra gruppelæreren om å rette opp
                d1_recv_data ved å kjøre valgrind med flagene.
            ####  

        d2_alloc_local_tree:
            allokerer minne for å lagre nodene

        d2_free_local_tree:
            frigjøre allokerte minne for nodene.


        d2_add_to_lokal_tree:
            legger til noder i den lokale treet.
            så sjekker om antall byt som er prosessert så lenge er mindre enn buflen,'
            så oppretter noden og koppierer den i bufferet og øker antall byt som er prosessert.

        d2_print_tree:
            itererer gjennom antall noder og printe hver node, så 
            itererer gjennon hver barnenode og printe det i en 2d for_loop.

# Hvilke elementer du har eller ikke har implementert?
    Jeg har implementert alle elementene som forventet.
    alle delere av oppgaven(d1_server, d1_dump, d2_server) fungerer som det skal.                      





















