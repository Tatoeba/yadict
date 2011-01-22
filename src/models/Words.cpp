#include <sstream>
#include "models/Words.h"

namespace models {

/**
 *
 */
Words::Words() {}

/**
 *
 */
TatoHyperItemFetcher* Words::getWordWithId(int id) {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();
    TatoHyperItem *word= tato_hyper_db_item_find(tatoHyperDb, id);

    TatoHyperItemFetcher *fetcher = tato_hyper_item_fetcher_new(1, 0);
    tato_hyper_item_fetcher_add(fetcher, word);

    return fetcher;
}


/**
 *
 */
TatoHyperItemFetcher* Words::getWordsWithStr(std::string str) {
    return getWordsWithStr(str, 10, 0);
}

TatoHyperItemFetcher* Words::getWordsWithStr(std::string str, int size, int offset) {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();

    TatoHyperItemFetcher *fetcher = tato_hyper_item_fetcher_new(size, offset);
    tato_hyper_db_items_search(tatoHyperDb, fetcher, str.c_str());

    return fetcher;
}


/**
 *
 */
TatoHyperItemFetcher* Words::getAllWords() {
    return getAllWords(10, 0);
}

TatoHyperItemFetcher* Words::getAllWords(int size, int offset) {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();

    TatoHyperItemFetcher *fetcher = tato_hyper_item_fetcher_new(size, offset);
    tato_hyper_db_items_get_seq(tatoHyperDb, fetcher);

    return fetcher;
}




/**
 *
 */
int Words::getRandomWordId() {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();
    TatoHyperItem *randWord = tato_hyper_db_item_rand(tatoHyperDb);

    if (randWord == NULL) {
        return 0;
    } else {
        return randWord->id;
    }
}

/**
 *
 */
TatoHyperItemFetcher *Words::getRandomWord() {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();
    TatoHyperItem *randWord = tato_hyper_db_item_rand(tatoHyperDb);

    TatoHyperItemFetcher *fetcher = tato_hyper_item_fetcher_new(1, 0);
    tato_hyper_item_fetcher_add(fetcher, randWord);

    return fetcher;
}

/**
 *
 */
bool Words::addWord(std::string lang, std::string str) {
    return addWord(lang, str, 0);
}

bool Words::addWord(
    std::string lang,
    std::string str,
    TatoHyperItemFlags flags
) {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();

    tato_hyper_db_item_add(
        tatoHyperDb,
        lang.c_str(),
        str.c_str(),
        flags
    );
    // TODO handle errors
    return true;
}

/**
 *
 */
bool Words::editWord(int id, std::string lang, std::string str) {
    return editWord(id, lang, str, 0);
}

bool Words::editWord(
    int id,
    std::string lang,
    std::string str,
    TatoHyperItemFlags flags
) {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();
    TatoHyperItem *word= tato_hyper_db_item_find(tatoHyperDb, id);
    if (word != NULL) {
        // update the lang
        // TODO maybe it's worth to check if the lang has actually changed
        // as well as string and flag 
        TatoHyperItemLang *tatoLang = tato_hyper_db_lang_find(
            tatoHyperDb,
            lang.c_str()
        );
        if (tatoLang != NULL) {
            tato_hyper_item_lang_set(word, tatoLang);
        }
        //update the string 
        tato_hyper_item_str_set(word, str.c_str());

        //update the flag
        tato_hyper_item_flags_set(word, flags);
    }
    // TODO handle errors
    return true;
}

/**
 *
 */

bool Words::deleteWord(int id) {
    TatoHyperDb *tatoHyperDb = TatoHyperDB::getInstance("")->getDatabasePointer();
    return tato_hyper_db_item_delete(tatoHyperDb, id);
}





}// end of models::Words