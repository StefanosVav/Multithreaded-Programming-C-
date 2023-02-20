Στέφανος Βάβουλας - ΕΡΓΑΣΙΑ 3

Η εργασία περιέχει:
-Απλό Makefile με δυνατοτητες make, make clean για δημιουργία του travelMonitorClient και του monitorServer
-input_dir που περιέχει country directories και files που κάνει parse η εφαρμογή
-Αρχεία κώδικα για την εφαρμογή: main.cpp, main2.cpp, citizen.cpp/h, BloomFilter.cpp/h SkipList.cpp/h date.cpp/h

Εντολές εκτέλεσης:
>make
>./travelMonitorClient -m 3 -b 10 -c 10 -s 1000 -i input_dir -t 5

Προφανώς το παραπάνω είναι παράδειγμα και οι μεταβλητές μπορούν να έχουν οποιεσδήποτε τιμές.
Η εργασία έχει αναπτυχθεί στα linux της σχολής μέσω PuTTY.


------> 50%: travelMonitorClients
Όσον αφορά το travelMonitorClient, έχω χωρίσει τη main σε 4 parts: 

---------------------------- PART 1: GET INPUT FROM COMMAND LINE AND SAVE IT ---------------------------------------
Απλός έλεγχος των command line arguments και ανάθεσή τους σε μεταβλητές.

-- PART 2: FORK NUM MONITORS CHILD PROCESSES AND EXECV MONITORSERVER FOR EACH ONE WITH THEIR RESPECTIVE ARGUMENTS --
Σε μία επανάληψη για numMonitors φορές, βρίσκω ποιες χώρες πρέπει να ανατεθούν στο συγκεκριμένο monitor με RoundRobin, κάνω fork και
το παιδί εκτελεί execv με port το port+i (όπου i ο αριθμός του κάθε monitorServer), και με paths τις χώρες που βρήκα ότι πρέπει να του
ανατεθούν.

------- PART 3: CONNECT TO MONITOR SERVERS THROUGH SOCKETS WITH ASSIGNED PORTS AND RECEIVE BLOOM FILTERS -----------
Σε μία επανάληψη για numMonitors φορές, γίνεται σύνδεση με κάθε monitorServer και δέχεται το όνομα των BloomFilters που έχουν 
αρχικοποιηθεί στον συγκεκριμένο monitorServer (Δεν κατάφερα να πετύχω το serialization και να περνάω ολόκληρα τα BloomFilters
από τα sockets οπότε περνάω μόνο τα ονόματα τους).

------------------------- PART 4: GET INPUT FROM USER AND PERFORM EACH COMMAND -------------------------------------
/exit: Έξοδος από την εφαρμογή. Το parent process στέλνει μία εντολή exit στους monitorServers μέσω socket και τυπώνει σε ένα 
αρχείο με ονομασία log_file.xxx όπου xxx είναι το process ID του, το όνομα όλων των χωρών που συμμετείχαν στην εφαρμογή, 
το συνολικό αριθμό αιτημάτων που δέχθηκε και το συνολικό αριθμό αιτημάτων που εγκρίθηκαν και απορρίφθηκαν.

/travelRequest: Στέλνει μήνυμα μέσω socket στο monitorServer που του έχει ανατεθεί η countryFrom να ελέγξει αν υπάρχει το συγκεκριμένο 
ID στα BloomFilters του και να επιστρέψει flag YES or NO. Στη συνέχεια το travelMonitor εκτυπώνει το αντίστοιχο μήνυμα και ενημερώνει
τις μεταβλητές Travel Request Counter, Accepted Requests Counter, Denied Requests Counter.

/travelStats: Εκτυπώνει τα στατιστικά Travel Request Counter, Accepted Requests Counter, Denied Requests Counter.

/searchVaccinationStatus: Σε μια επανάληψη στέλνει μέσω socket σε κάθε monitorServer το ID που ζητείται, τα monitorServers ψάχνουν 
στα SkipLists τους το συγκεκριμένο citizen, και αν τον βρουν εκτυπώνουν όλα τα δεδομένα του.


------> 50%: monitorServer
Όσον αφορά το monitorServer, έχω χωρίσει τη main2 σε 5 parts: 

------------------------------- PART 1: GET INPUT FROM EXECV AND SAVE IT -------------------------------------------
Απλός έλεγχος των arguments που δέχεται από την execv και ανάθεσή τους σε μεταβλητές.

------------------- PART 2: INITIALIZE DATA STRUCTURES BY PARSING THROUGH ASSIGNED FILES ---------------------------
Μπαίνει στα paths - χώρες που του έχουν ανατεθεί και αρχικοποιεί citizen list, bloom filters list και skip lists list με τα 
records στα .txt files.

------- ------------ PART 3: INITIALIZING CONNECTION THROUGH SOCKETS WITH PARENT-CLIENT ----------------------------
Ο monitorServer αρχικοποιεί τη σύνδεση με τον parent-client (socket, bind, listen, accept)

---------------------- PART 4: SERIALIZING AND SENDING BLOOM FILTERS TO PARENT PROCESS -----------------------------
Δεν κατάφερα να πετύχω το serialization οπότε στέλνει στον parent τα ονόματα από όλα τα BloomFilters που έχει αρχικοποιήσει.

-----------------PART 5: RECEIVE COMMANDS-REQUESTS FROM PARENT-CLIENT AND REACT ACCORDINGLY ------------------------
Παίρνει τις εντολές μέσω socket από τον parent-client και κάνει τις αντίστοιχες ενέργειες. Όταν δεχτεί εντολή exit, κλείνει το
socket, απελευθερώνει τη μνήμη από όλες τις δομές δεδομένων και τερματίζει.

Τυχόν λεπτομέρειες για την υλοποίηση αναφέρονται στα σχόλια του κώδικα.

