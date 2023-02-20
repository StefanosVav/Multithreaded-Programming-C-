Στέφανος Βάβουλας - ΕΡΓΑΣΙΑ 2

Η εργασία περιέχει:
-Απλό Makefile με δυνατοτητες make, make clean για δημιουργία του travelMonitor
-inputFile.txt, countriesFile.txt αρχεία με δεδομένα που χρησιμοποιούνται για δημιουργία input_dir
-create_infiles.sh bash που δημιουργεί το input_dir και το γεμίζει με αρχεία και εγγραφές από τα παραπάνω αρχεία
-Αρχεία κώδικα για την εφαρμογή: main.cpp, main2.cpp, citizen.cpp/h, BloomFilter.cpp/h SkipList.cpp/h date.cpp/h

Εντολές εκτέλεσης:
Για εκτέλεση του bash script/δημιουργία input_dir
>./create_infiles.sh inputFile.txt input_dir 3
>g++ -o Monitor main2.cpp citizen.cpp SkipList.cpp BloomFilter.cpp date.cpp
>make
>./travelMonitor -m 3 -b 40 -s 1000 -i input_dir

Προφανώς οι αριθμοί numMonitors και numFilesPerDirectory μπορούν να έχουν οποιαδήποτε τιμή.
Η εργασία έχει αναπτυχθεί στα linux της σχολής μέσω PuTTY.

--> 10%: bash script
Όσον αφορά το bash script: Παίρνει τα arguments, ελέγχει για ήδη ύπαρξη του input_dir, δημιουργεί directories και .txt files για
κάθε χώρα και στη συνέχεια γεμίζει τα αρχεία κάθε χώρας με records από το inputFile με RoundRobin αλγόριθμο. Για να τον υλοποιήσω,
δημιούργησα ένα Round Robin array το οποίο κρατά στο index i που αντιστοιχεί σε κάθε χώρα(directory) τον αριθμό του επόμενου text
file στο οποίο θα βάλουμε το επόμενο record απύ αυτή τη χώρα. Αρχικά, θέτουμε όλα τα στοιχεία του array ίσα με 1 και προσθέτουμε 1
σε κάθε εισαγωγή record. Όταν ξεπεράσει το numFilesPerDirectory, θέτω τον αριθμό ξανά σε 1.
Τυχόν λεπτομέρειες για την υλοποίηση αναφέρονται στα σχόλια του κώδικα.

--> 90%: travelMonitor
Όσον αφορά το travelMonitor, έχω χωρίσει τη main σε 3 parts: 
----- PART 1: GET INPUT FROM COMMAND LINE (INPUT FILE NAME, BLOOM SIZE) AND SAVE IT -----
Απλός έλεγχος των command line arguments και ανάθεσή τους σε μεταβλητές

--- PART 2: CREATE NAMED PIPES FOR COMMUNICATION BETWEEN NUM MONITORS CHILD PROCESSES ---
Δημιουργώ ένα directory NP μέσα στο οποίο ορίζω τα Named Pipes για επικοινωνία μεταξύ της εφαρμογής και των Monitors. 
Δημιουργώ μόνο ένα fifo μόνο για κάθε Monitor μέσω του οποίου κάνω τις send και τις receive.
Επιπλέον δημιουργώ ένα FIFOARRAY που σώζει στα index i (i έως numMonitors) το path του i-οστού fifo.

-------------- PART 3: FORK NUM MONITORS CHILD PROCESSES AND EXEC MONITOR FOR EACH ONE WITH THE NAMED PIPES AS ARGUMENTS --------------------
Σε μια επανάληψη, εφαρμόζει fork numMonitors φορές. Στο child process καλεί την exec (δηλαδή το executable Monitor) ενώ στο parent process,
σώζει τα pid κάθε παιδιού σε ένα πίνακα PIDARRAY.

--- PART 4: OPEN THE NAMED PIPES AND ASSIGN COUNTRY DIRECTORIES TO THE CHILD-MONITORS ---
Ανοίγω τα named pipes και το input_dir και στέλνω (αντίστοιχα με τον Round Robin αλγόριθμο στο bash) μέσω των named files σε κάθε Monitor τις χώρες 
που του αντιστοιχούν.

--- PART 5: GET SERIALIZED BLOOM FILTERS FROM THE CHILD-MONITORS AND DESERIALIZE THEM ---
Υλοποίησα serialize και deserialize functions στα BloomFilters αλλά δεν κατάφερα να υλοποίησω τη μεταφορά των serialized bloom filters από τα
Monitors στο travelMonitor.

------------- PART 6: GET INPUT FROM USER AND PERFORM EACH COMMAND ----------------------
/exit: Στέλνει SIGKILL σε όλα τα Monitors και κάνει break από το loop του get input οπότε τερματίζει και η travelMonitor.
/travelRequest: Εφόσον δεν μπόρεσα να στείλω πίσω τα Bloom Filters από τα Monitors, δεν μπορώ να ελέγξω στην travelMonitor αν υπάρχει το συγκεκριμένο
ID σε BloomFilter. Οπότε, στέλνω μήνυμα στο Monitor που του έχει ανατεθεί η countryFrom να ελέγξει αν υπάρχει το συγκεκριμένο ID στα BloomFilters του
και να επιστρέψει flag YES or NO. Στη συνέχεια το travelMonitor εκτυπώνει το αντίστοιχο μήνυμα.

Τυχόν λεπτομέρειες για την υλοποίηση αναφέρονται στα σχόλια του κώδικα.

