# Hvordan du leser master file table-filen  fra disk og laster inodene inn i minnet.
åpner "master_file_table" ved fopen og allokere minne med en double peker som skal lagre hver inode peker. siden vi ikke vet hvor langt er filen, bruker while_loop. ved bruke av fread leser 4 bytes for id, så neste 4 bytes for navnLengde som siste byten sier om hvor langt er navnet, så leser inn navnet, neste byte leses is_directory, deretter forsetter med neste byte som er is_readonly, så leses neste 4 bytes som file_size og deretter leses num_entries(siste byte sier om hvor mange entries skal leses inn) i de neste 4 bytes, så etter det leser vi 8 bytes in 64 bit OS pga(Uintptr_t) som er entries.


# Eventuelle implementasjonskrav som ikke er oppfylt.
Vi har implementert alle funksjonene vi skulle, load_inodes, create_dir, create_file, find_inode_by_name, fs_shutdown.  

# Eventuelle deler av implementasjonen som avviker fra prekoden. Dersom du for eksempel oppretter egne filer, forklar hva hensikten deres er.
Har ikke opprettet noen ekstra filer(bortsatt fra makefile) eller funksjoner, men hadde en avvik fra prekoden hvor vi kommenterte inn (//if( error == 0 || error == ENOENT )) inn i allocation.c for å unngå warning unused variable, så fikk ikke til å lese koden når vi skulle kjøre (make valgrind_create2 og  make valgrind_create3). fikset det ved hjelpe av gruppelærerveiledningen.

# Eventuelle tester som feiler og hva du tror årsaken kan være.
Vi hadde minnelekkasje og error i valgrind_create2 og valgrind_create3 og grunnen var at noen pekere som har blitt mikset rundt og mistet minneområdet de har opprinnelig blitt satt til. den ble fikset ved hjelpe av veiledningen til gruppelærer.