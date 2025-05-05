# Formalne jazyky a prekladace - Projekt

## Implementacia prekladaca jazyka IFJ2021

### Autori:
  - Adam Dzurilla (xdzuri00)
  - Martin Mores (xmores02)
  - Jakub Kasem (xkasem02)
  
----------------------------------------------------------------------------------------------------------------------------
### Subory:
    - Rozdelenie bodov pre jednotlivych pracantov na tomto projekte     => rozdelenie
    - Implementacia tabulky symbolov                                    => symtable.h, symtable.c
    - Zoznamy typov a tabuliek symbolov                                 => symtable.h, symtable.c
    - Lexikalny analyzator                                              => scanner.h, scanner.c
    - Syntakticky analyzator                                            => parser.h, parser.c
    - Spracovanie vyrazov                                               => expression.h, expression.c
    - Zasobnik pre spracovanie vyrazov                                  => stack.h, stack.c
    
### Lexikalny analyzator
  Lexikalny analyzator dostava na vstup postupnost znakov a na zavolanie funkcie get_token vracia posledny este nenacitany token, funkcia moze vratit nasledujuce typy tokenov: identifikator, klucove slovo, celociselny literal, desatinny literal, retazcovy literal, operator plus, operator minus, operator krat, operator delenie, operator celociselne delenie, relacny operator vacsi, relacny operator mensi, relacny operator vacsi rovny, relacny operator mensi rovny, relacny operator rovny, relacny operator rozny, konkatenacia retazcovych literalov a hashtag
  
### Konecny automat pre urcovanie tokenov
  Konecny automat stavov je navrhnuty aby rozlisoval ci postupnost znakov na vstupe je validna pre dany jazyk. Rozhoduje o aky typ tokenu sa jedna a vracia dany token syntaktickemu analyzatoru spolu s dalsimi atributmi.

### Syntakticky analyzator
  Syntakticky analyzator bol navrhnuty podla dopredu vypracovanej LL Gramatiky, syntakticky analyzator funguje rekurzivnou metodou zostupu. Pre kazdy neterminal LL Gramatiky bola vytvorena funkcia ktora ma jasne definovane pravidla pre dalsie tokeny. V pripade ze syntakticky analyzator dostane token pre ktory nepozna prechod, vracia syntakticku chybu (chybovy kod 2). V pripade ze syntakticky analyzator ktory ma epsilon prechod dostane neocakavany token, vrati ho naspat lexikalnemu analyzatoru pomocou funkcie return_token, a vracia uspesnost svojej operacie. Chybovy kod sa vracia pomocou parametru return_code nachadzajuci sa v strukture SPars_data ktora je predavana medzi funkciami neterminalov. Deklaracia struktury SPars_data sa nachadza v subore "parser.h". Struktura obsahuje aj ukazatel na token do ktoreho je nacitany vzdy nasledujuci token aby sme nemuseli zakazdym alokovat novy token.

### LL Gramatika
  LL Gramatika bola navrhnuta podla zadani v subore "/docs/ifj2021.pdf". Pociatocny stavom je <prol> kde je vyzadovana postupnost tokenov " require \"ifj21\" ". Po naciatni tohto prologu prechazda syntakticky analyzator do stavu <prog> kde moze nacitavat - Deklaracie funkcii, Definicie funkcii, Volania funkcii a token ukoncenia suboru. Ukoncovaci token ukonci syntakticku analyzu. LL Gramatika sa sklada este zo stavov <return_func_var> ktory pomaha urcit navratove hodnoty funkcie, <params> ktory pomaha nacitat parametre pri definicii funkcie, <param_n> ktory sluzi pre nacitanie viac ako jedneho parametru funkcie, <statement> ktory rozsiruje nacitanie viacerych prikazov - inicializacia premennej (pociatocny token KEYWORD_LOCAL), vracanie hodnot pri vystupe z tela funkcie (pociatocny token KEYWORD_RETURN), priradenie hodnot do jedneho alebo viacerych identifikatorov (pociatocny token IDENTIFIKATOR), podmieneny prikaz if (pociatocny token KEYWORD_IF) a cyklus while (pociatocny token KEYWORD_WHILE). Prechod pre gramatiku pomocou neterminalu <assign_value> ktora pomaha pri urceni ci definicii identifikatoru priradime hodnotu alebo ho nechame nedefinovanym. Prechod pomocou neterminalu <id_n> urcuje ci sme uz skoncili s nacitavanim identifikatorov pre priradenie alebo nie. Prechod pomocou neterminalu <types> urcuje ci ideme nacitavat postupnost typov alebo nie. Prechod pomocou neterminalu <type_n> urcuje ci sme uz skoncili s nacitavanim typov alebo nie. Prechod pomocou neterminalu <type>: ocakavame jeden z tokenov KEYWORD_INTEGER, KEYWORD_NUMBER, KEYWORD_STRING alebo KEYWORD_NIL. Prechod pomocou neterminalu <assig_func> urcuje ci islo o volanie funkcie alebo o nacitavanie premennych. Cela LL Gramatika sa nachadza bud v subore "/docs/Dokumentacia.docx" alebo v subore "/docs/dokumentacia.pdf".

### LL Tabulka
  LL Tabulka bola vypracovana pomocou LL Gramatiky a urcuje prechody z neterminalov do inych stavov pomocou tokenov, v pripade ze danym tokenom neexistuje prechod jedna sa o syntakticku chybu. Cela LL Tabulka sa nachadza bud v subore "/docs/LL_tabulka.xlsx" alebo v subore "/docs/dokumentacia.pdf".

### Semanticka analyza
  Semanticka analyza bola implementovana spolu so syntaktickou analyzou vo funkciach neterminalov podla potreby vykonania semantickej kontroly. Pomocne premenne sa nachadzaju v strukture SPars_data definovanej v subore "parser.h". Semanticka analyza si pamata stav v ktorom sa nachadza napr. deklaracia funkcie, definicia funkcie, volanie funkcie a podobne. Vsetky stavy sa nachadzaju v enum triede Parser_State deklarovanej v subore "parser.h". Dalej si semanticka kontrola pamata ci sa nacitavaju typy parametrov alebo typy navratovych hodnot. Tato premenna je pouzivana pri deklaracii alebo definicii funkcii. Struktura dalej obsahuje globalnu tabulku symbolov v ktorej su ulozene funkcie programu, a zoznam lokalnych tabuliek symbolov. Lokalna tabulka symbolov je vytvorena pri definicii funkcie a hned su do nej vlozene parametry funkcie, dalsia uroven tabulky sa vytvori pri prikazoch IF alebo WHILE. Struktura este obsahuje aj zoznam typov do ktoreho sa nacitavaju postupne typy pri priradeni do identifikatorov, tento zoznam je vyuzity pri kontrole typovej kompatibility vo funkcii func_expression. Pri urceni kde sa bude kontrolovat semanticka kontrola sme si pomohli LL Gramatikou.

### Testovacie subory
  Pocas nasho vyvoja prekladaca sme si vytvorili viacero testov na kontrolu rozncyh casti prekladaca. Vsetky testy sme situovali do zlozky "/tests".
  1. Testy pre lexikalnu analyzu ("/tests/test_lexical_analysis.c") dostanu subor a vypisuju tokeny na standardny vystup pod seba spolu s atributom ak nejaky atribut tieto tokeny maju.
  2. Testy pre syntakticku analyzu ("/tests/test_syntactic_analysis.c") dostanu subor a zavolaju funkciu func_prol ktorej ako argument daju strukturu SPars_data, funkcia prechadza program a syntakticky kontroluje jeho spravnost, v pripade ze navratovy kod je NO_ERROR (podla suboru error.h) je vypisany na standardny vystup text oznacujuci uspesnost operacie, v pripade ze navratovy kod nie je NO_ERROR je vypisany navratovy kod aby sme vedeli o aky typ chyby ide.
  3. Testy pre tabulky symbolov - test vytvori tabulku symbolov pre funkcie a tabulku symbolov pre premenne. Postupne su otestovane funkcie pracujuce s tymito tabulkami rovnako ako vymazavania tychto tabuliek.

### Pouzite algoritmy z predmetu IAL
  Pouzili sme nase vedomosti k tvorbe tabulky symbolov kde sme implementovali binarny vyhladavaci strom (podla nasho zadania projektu - varianta I), pri binarnom strome sme pouzili funkcie pre - inicializaciu, vkladanie, hladanie a odstranenie celeho stromu. Ostatne funkcie pre pracu s binarnym stromom sme nepotrebovali tak sme ich neimplementovali. Dalej sme si definovali funkcie pre vytvorenie funkcii a premennych ktore alokuju miesto a inicializuju hodnoty. takisto sme si vytvorili aj funkcie pre odstranenie dat z binarneho vyhladavacieho stromu ktore korektne uvolnia alokovany priestor. Potom sme si este implementovali zasobnik pre riesenie vyrazov pomocou postifsovej notacie

  Pri generovani vyrazov sme vyuzili znalosti z generovania postfixovych vyrazov

### Generovanie kodu
  Generovanie kodu sme implementovali v suboroch code_generator.c a code_generator.h. Pri generovani kodu sme zvolili pristup vyuzitita funkcii ktore generuju kod do symboloveho retazca a ten sa po uspesnej syntaktickej a semantickej kontrole vypise na standardny vystup

### Prekazky s ktorymi sme sa stretli
  1. Zle sme si odhadli cas ktory bude na projekt potreba takze sme ho robili na poslednu chvilu, co malo za nasledok mozno nie uplne dokonalu implementaciu projektu
  2. Nedostatocne prvotne pochopenia zadani v lexikalnej analyze kde sme prevadzali hashtag na INT cislo hned v lexikalnej analyze, potom sme sa dozvedeli ze to moze byt zadane aj vo forme #s1 kde s1 je identifikator, dalej sme pridavali funkciu return_token v lexikalnej analyze ktorej implementacia nam trosku robila problemy, a pri vracani tokenu identifikatoru sme nechtiac vracali aj uvodzovky co pri naslednom nacitani nenacitalo identifikator ale string a s odhalenim tejto chyby sme takisto stravili nejaky cas.
  3. Upravy LL Gramatiky sme nastastie vyriesili velmi dobrou implementaciou kde najvacsie upravy bolo treba robit v komentaroch a syntakticka analyza bola pre nas velmi prehladna a lahko upravovatelna na zaklade pravidiel ktore sme si vytvorili
  4. S vela upravami nam pomohli aj testy ktore sme si vytvorili a testovacie subory ktore sme si bud vycitali z textu, dostali ich alebo sme si vytvorili vlastne
  5. Moresko

### Pomocky pri projekte
  Pouzili sme vypis pri chybach prekladaca na standardny chybovy vystup pre lahsiu detekciu v ktorej casti prekladu doslo k neuspechu

### Tymova praca
  Moresko spravil vsetko
  1. Dzuris - Lexikalna analyza, Syntakticka analyza, Tabulky symbolov, Semanticka analyza, Testy, Dokumentacia, Struktura projektu, LL Gramatika, LL Tabulka, Konecny automat pre nacitavanie tokenov, Zoznamy typov, Zoznamy tabuliek symbolov
  2. Kubo bot - Syntakticka analyza, Syntakticka analyza vyrazov, Semanticka analyza vyrazov, LL Gramatika, LL Tabulka, Dokumentacia, Precedencna tabulka
  3. 1001101 (To je M ako moresko) - Precedencna tabulka, Dokumentacia, Generovanie kodu, Tabulka symbolov

### Zhrnutie
  Projekt nam dal vela znalosti v oblastiach timovej prace kde sme sa naucili ako kooperovat pri implementaciach, ako si rozdelit ulohy medzi sebou a ako komunikovat pri vytvarani roznych pomocnych strktur, ako postupovat spolocne pri implementaciach

### FUNEXP
  Pri projekte sme si vybrali rozsirenie funexp, rozsirenie sme implementovali primarne v subore expression.c, kde sme si zvolili pri volani funkcie ako parametre <expression>, a pri kazdom parametre volame funkciu <func_expression> rekurzivne. Pri expression sme si vytvorili pravidlo ktore moze citat po sebe iduce tokeny id a lava zatvorka, na zaklade tohto pravidla vieme ze ide o volanie funkcie pri expressions