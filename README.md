# Λειτουργικά Συστήματα
## **Εργασία 1**
## Σταυρούλα Χριστοπούλου, 1115201800213

>### **Εντολή μεταγλώττισης**: <code> make </code>
(Έχει υλοποιηθεί αρχείο Makefile)

>### **Εντολή εκτέλεσης**: <code> ./server X_Filename K_Clients N_Transactions </code>
**όπου**:
- **X_Filename**: Το αρχείο στον *φάκελο txt* που θα χρησιμοποιηθεί για την επιστροφή των αιτήσεων των παιδιών στην γονική διεργασία.
- **K_Clients**: Αριθμός πελατών/παιδιών διεργασιών
- **N_Transactions**: Αριθμός δοσοληψιών παιδιών

>### **Δομή project και Διαχωρισμός αρχείων:**
Για λόγους απλούστευσης του κώδικα, έχει υλοποιηθεί ένα interface, με τα παρακάτω directories και αρχεία:
- **src:**
    - **semaphores.c**: Υλοποιήση συναρτήσεων *system v* ***σημαφόρων***, για την δημιουργία, αρχικοποίηση, καταστροφή, και block/unblock των σημαφόρων για την κατάλληλη **IPC**.
    - **shared_memory.c**: Υλοποίηση συναρτήσεων ***διαμοιραζόμενης μνήμης***, για την δημιουργία, την καταστροφή, την σύνδεση και την αποσύνδεση της.

- **include**: header files για τα παραπάνω αρχεία των σημαφόρων και της διαμοιραζόμενης μνήμης. 
- **txt**: Ο φάκελος με το **αρχείο Χ** που θα χρησιμοποιηθεί από τον εξυπηρετητή(**server**), για την επιστροφή των ζητούμενων γραμμών στους πελάτες(**clients**).

- ./**Makefile:** Για την μεταγλώττιση και τη σύνδεση όλων των αρχείων.

>### **server.c**:
Κατά την μεταγλώττιση, από το αρχείο αυτό παράγεται και το εκτελέσιμο. Έχει σχεδιαστεί κατάλληλα η δομή διαμοιραζόμενης μνήμης καθώς και το απαιτούμενο σύνολο σημαφόρων, για την επικοινωνία της διεργασίας-πατέρας με τις διεργασίες-παιδιά, και να ικανοποιεί σωστά και αποτελεσματικά τα αιτήματα τους.

**Η λειτουργία του**:
1. Λαμβάνονται από τον χρήστη μέσω του command line: το ζητούμενο αρχείο **Χ**, το πλήθος των διεργασιών-παιδιών **Κ**, καθώς και το πλήθος των δοσοληψιών **Ν** στις οποίες αυτά θα πάρουν μέρος.
2. *Αρχικοποιούνται* οι **σημαφόροι** που θα χρησιμοποιηθούν και η δομή της **διαμοιραζόμενης μνήμης**, η οποία και συνδέεται με το πρόγραμμα. Επίσης αρχικοποιείται η δομή της διαμοιραζόμενης μνήμης από τον πατέρα, με τον αριθμό των γραμμών του αρχείου που έδωσε ο χρήστης.
3. Σε ένα for loop, μέσω της ***fork()***, ο πατέρας/εξυπηρετητής δημιουργεί τα παιδιά του, όσα και του δόθηκαν από τον χρήστη, και ***συνεχίζει μόνο αφότου*** έχουν δημιουργηθεί ***όλα***(κατάλληλο handling του σημαφόρου **mutex** ο οποίος μπλοκάρεται αρχικά, και ξεμπλοκάρεται μόνο όταν ο πατέρας βγεί από το loop).
4. Στην συνέχεια ελέγχεται αν βρίσκόμαστε σε διεργασία-παιδί(pid == 0) ή διεργασία-πατέρα(pid != 0), όπου και η εκτέλεση γίνεται για όσες φορές γνωστοποιήθηκε από τον χρήστη, και με την κατάλληλη χρήση των σημαφόρων, που θα εξηγηθεί παρακάτω, ο πελάτης λαμβάνει την γραμμή που ζήτησε, από τον εξυπηρετητή.
5. Οι γραμμές που επιστρέφονται από τον πατέρα, και ο μέσος χρόνος απόκρισης στην κάθε αίτηση που λαμβάνεται, γράφονται για κάθε διεργασία-παιδί με το ID του, στα αρχεία ***requests.log & avg_times.log*** αντίστοιχα.
6. Ο πατέρας περιμένει μέσω της **wait()**, για να τερματίσουν όλα τα παιδιά του.
7. Γίνεται **free_resources()**, δηλαδή αποσυνδέεται και ελευθερώνεται η μνήμη, καθώς και οι 3 σημαφόροι που χρησιμοποιήθηκαν.

>### Σημαφόροι & Επικοινωνία Γονέα-Παιδιού
- semaphore **mutex**: Χρησιμοποιείται για τον ***αμοιβαίο αποκλεισμό των υπόλοιπων clients***, όσο ένας client είναι στο *Critical Section* του, καθώς και για τον αποκλεισμό/block των παιδιών, εφόσον ο πατέρας δεν έχει δημιουργήσει ακόμα όλα τα παιδιά-clients. Είναι αρχικοποιημένος με 1.
- semaphore **client**: Χρησιμοποιείται για τον αποκλεισμό του server, όταν ο client είναι στο CS. Είναι αρχικοποιημένος με 0.
- semaphore **server**: Χρησιμοποιείται για τον αποκλεισμό του client, όταν ο server είναι στο CS. Είναι αρχικοποιημένος με 0.

*Οι σημαφόροι client και server είναι αρχικοποιημένοι με 0, ώστε αρχικά να μπλοκαριστεί ο πατέρας όταν το παιδί θα κάνει την αίτηση, και ύστερα να μπλοκαριστεί το παιδί για να λάβει την αίτηση ο πατέρας και να στείλει την απάντηση του με την γραμμή από το αρχείο.*

>Η λογική πίσω από την επικοινωνία του γονέα με το παιδί, και της κατάλληλης ενημέρωσης των δεδομένων της διαμοιραζόμενης μνήμης, είναι να μπλοκάρεται ο ένας από τους δύο, και μόνο όταν έχει ενημερώσει την μνήμη κατάλληλα ο άλλος, να τον ξεμπλοκάρει.

### Πιο συγκεκριμένα:
1. Αρχικά, ο πατέρας ενημερώνει τα δεδομένα της μνήμης με τον αριθμό των γραμμών του αρχείου.
2. Το παιδί πρέπει να στείλει την αίτηση του στον πατέρα μέσω της διαμοιραζόμενης μνήμης(να ενημερώσει μία μεταβλητή αριθμού γραμμής με ένα τυχαίο αριθμό), γι'αυτό και πρέπει ο πατέρας να μπλοκαριστεί.
    - Εάν είμαστε στον πατέρα(pid != 0), προσπαθεί να κατεβάσει τον σημαφόρο του server(sem_down(server)), ωστόσο αποτυγχάνει επειδή είναι αρχικοποιημένος με 0, και μπλοκάρεται. Τρέχει λοιπόν το παιδί...
    - Αν είμαστε στο παιδί(pid == 0), στέλνει κανονικά την αίτησή του, εφόσον ο πατέρας είναι μπλοκαρισμένος, αφού όμως έχει κατεβάσει τον σημαφόρο mutex, για τον αποκλεισμό των υπόλοιπων παιδιών μέχρι να ολοκληρωθεί η τρέχουσα δοσοληψία.
    - Σε αυτό το σημείο αρχίζει και μετράει ο χρόνος μέχρι την λήψη απάντησης από τον πατέρα.
3. Εφόσον το παιδί έστειλε την αίτηση του, και πρέπει να λάβει την απάντηση από τον πατέρα, πρέπει να τον ξεμπλοκάρει. Ανεβάζει λοιπόν τον σημαφόρο του πατέρα server, ώστε να του επιτρέψει να τρέξει(sem_up(server)), και κατεβάζει τον δικό του σημαφόρο client (sem_down(client)). Επειδή όμως έιναι αρχικοποιημένος σε 0, μπλοκάρεται και περιμένει τον πατέρα.
4. Ο πατέρας με την sem_down(server) πλέον μπορεί να τρέξει, και έτσι ανοίγει το αρχείο του χρήστη, και με την fgets() βρίσκει την γραμμή που ζήτησε το παιδί. Με αυτήν ενημερώνει τα δεδομένα της διαμοιραζόμενης μνήμης. Παράλληλα ξεμπλοκάρει το παιδί, ανεβάζοντας τον σημαφόρο client (sem_up(client)).
5. Το παιδί πλέον μπορεί να τρέξει και πάλι, όπου λαμβάνει την αιτηθείσα γραμμή, και την αποθηκεύει στο αρχείο requests.log
6. Αφού το παιδί έχει τελειώσει με την τρέχουσα δοσοληψία, ανεβάζει τον σημαφόρο mutex, και μπορούν πλέον να διεκδικήσουν τον
πόρο και τα υπόλοιπα παιδιά.
7. Αφού ολοκληρωθούν και οι N δοσοληψίες του παιδιού, υπολογίζεται ο μέσος χρόνος απόκρισης και αποθηκεύεται στο αρχείο avg_times.log

## Σημειώσεις/Παραδοχές
- Η διαμοιραζόμενη μνήμη αποτελείται από 3 μεταβλητές, μέσω των οποίων επικοινωνούν ο πατέρας και τα παιδιά.
    1. Αριθμός γραμμών αρχείου.
    2. Αριθμός τυχαία επιλεγμένης γραμμής, από το παιδί.
    3. Επιστρεφόμενη αιτηθείσα γραμμή του αρχείου από τον πατέρα.
- Οι γραμμές που επιστρέφονται από τον πατέρα, και ο μέσος χρόνος απόκρισης στην κάθε αίτηση που λαμβάνεται, γράφονται για κάθε διεργασία-παιδί με το ID του, στα αρχεία requests.log & avg_times.log αντίστοιχα.
