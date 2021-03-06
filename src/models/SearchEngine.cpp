#include "SearchEngine.h"
#include "generics/Languages.h"
#include <cmath>


SearchEngine::SearchEngine() {

    ISOToNameMap isoToName = Languages::get_instance()->get_iso_to_name_map();
    ISOToNameMap::const_iterator itr;
    for (itr = isoToName.begin(); itr != isoToName.end(); ++itr) {
        add_index(itr->first);
    }


    // create the object
}

SearchEngine::~SearchEngine() {
    // close the databases
    //

    LangsDbsMap::const_iterator end = langsDbs.end();
    for (LangsDbsMap::const_iterator it = langsDbs.begin(); it != end; ++it) {
        if (!tcidbclose(it->second)) {
            errorCode = tcidbecode(it->second);
            std::cerr << "close error: "<< tcidberrmsg(errorCode) << std::endl;
        }
        // delete the object
        tcidbdel(it->second);
    }
}

/**
 *
 */

void SearchEngine::init_indexed_metas(
    cppcms::json::object langsIndexedMetasJson
) {

    ISOToNameMap isoToName = Languages::get_instance()->get_iso_to_name_map();
    ISOToNameMap::const_iterator itr;
    for (itr = isoToName.begin(); itr != isoToName.end(); ++itr) {
        
        std::string tempLang = itr->first;
        if (langsIndexedMetasJson[tempLang].is_null()) {
            continue;
        }
        cppcms::json::array indexedMetas = langsIndexedMetasJson[tempLang].array();
        for (
            cppcms::json::array::const_iterator jsonItr = indexedMetas.begin();
            jsonItr != indexedMetas.end();
            ++jsonItr
        ) {
            std::string meta = (*jsonItr).str();
            langsIndexedMetas[tempLang].push_back(meta);
            add_index(tempLang + "_" + meta);
        }
    }
}

/**
 *
 */
void SearchEngine::add_index(std::string indexName) {

        TCIDB *tempIDB = tcidbnew();
        tcidbsetcache(tempIDB,-1,-1);
        // open the database
        if (!tcidbopen(tempIDB, indexName.c_str(), IDBOWRITER | IDBOCREAT | IDBOLCKNB )) {
            errorCode = tcidbecode(tempIDB);
            std::cerr << "open error: "<< tcidberrmsg(errorCode) << std::endl;
        }
        langsDbs[indexName] = tempIDB;
}

/**
 *
 */
void SearchEngine::add_word(
    int wordId,
    std::string text,
    std::string lang
) {
    if (langsDbs.find(lang) == langsDbs.end()) {
        return;
    }

    if (!tcidbput(langsDbs[lang], wordId, text.c_str())) {
        errorCode = tcidbecode(langsDbs[lang]);
        std::cerr << "add error: "<< tcidberrmsg(errorCode) << std::endl;
    }
}

/**
 *
 */
void SearchEngine::add_meta(
    int wordId, 
    std::string key,
    std::string value,
    std::string lang
) {
    add_word(
        wordId,
        value,
        lang + "_" + key
    );
}


/**
 *
 */
void SearchEngine::edit_word(
    int wordId,
    std::string oldText,
    std::string newText,
    std::string oldLang,
    std::string newLang
) {
    if (
        langsDbs.find(oldLang) == langsDbs.end() ||
        langsDbs.find(newLang) == langsDbs.end()
    ) {
        return;
    }

    if (!tcidbout(langsDbs[oldLang], wordId)) {
        errorCode = tcidbecode(langsDbs[newLang]);
        std::cerr << "remove error: "<< tcidberrmsg(errorCode) << std::endl;
        return;
    }


    if (!tcidbput(langsDbs[newLang], wordId, newText.c_str())) {
        errorCode = tcidbecode(langsDbs[newLang]);
        std::cerr << "add error: "<< tcidberrmsg(errorCode) << std::endl;
        return;
    }
}

/**
 *
 */
void SearchEngine::edit_lang(
    int wordId,
    std::string text,
    std::string oldLang,
    std::string newLang
) {
    
}

/**
 *
 */
void SearchEngine::edit_meta(
    int wordId, 
    std::string key,
    std::string value,
    std::string lang
) {
    std::string indexName(lang + "_" + key);
    if (langsDbs.find(indexName) == langsDbs.end()) {
        return;
    }

    if (!tcidbout(langsDbs[indexName], wordId)) {
        errorCode = tcidbecode(langsDbs[indexName]);
        std::cerr << "remove error: "<< tcidberrmsg(errorCode) << std::endl;
        return;
    }


    if (!tcidbput(langsDbs[indexName], wordId, value.c_str())) {
        errorCode = tcidbecode(langsDbs[indexName]);
        std::cerr << "add error: "<< tcidberrmsg(errorCode) << std::endl;
        return;
    }

}



/**
 *
 */
void SearchEngine::remove_word(
    int wordId,
    std::string lang
) {
    if (langsDbs.find(lang) == langsDbs.end()) {
        return;
    }

    if (!tcidbout(langsDbs[lang], wordId)) {
        errorCode = tcidbecode(langsDbs[lang]);
        std::cerr << "remove error: "<< tcidberrmsg(errorCode) << std::endl;
    }

    // remove also the metas from the indexes
    IndexedMetas::const_iterator end = langsIndexedMetas[lang].end();
    for (
        IndexedMetas::const_iterator itr = langsIndexedMetas[lang].begin();
        itr != end;
        ++itr
    ) {
        std::string indexName(lang + "_" + *itr);
        if (!tcidbout(langsDbs[indexName], wordId)) {
            errorCode = tcidbecode(langsDbs[indexName]);
            std::cerr << "remove error: "<< tcidberrmsg(errorCode) << std::endl;
        }
       
    }

}


/**
 *
 */
void SearchEngine::remove_meta(
    int wordId,
    std::string key,
    std::string value,
    std::string lang
) {
    std::string indexName(lang + "_" + key);
    if (langsDbs.find(indexName) == langsDbs.end()) {
        return;
    }

    if (!tcidbout(langsDbs[indexName], wordId)) {
        errorCode = tcidbecode(langsDbs[indexName]);
        std::cerr << "remove error: "<< tcidberrmsg(errorCode) << std::endl;
        return;
    }


}

SearchResults SearchEngine::search(
    std::string request,
    std::string lang,
    int size,
    int offset
) {
    std::set<int> uniqueResults;
    SearchResults results;


    search_one_index(
        request,
        lang,
        size,
        offset,
        results,
        uniqueResults
    );

    IndexedMetas::const_iterator end = langsIndexedMetas[lang].end();
    for (
        IndexedMetas::const_iterator it = langsIndexedMetas[lang].begin();
        it != end && (size - results.size()) > 0; 
        ++it
    ) {
        search_one_index(
            request,
            lang + "_" + *it,
            size - results.size(),
            std::max(0, offset- (int)uniqueResults.size()),
            results,
            uniqueResults
        );
        SearchResults::iterator insertIt = results.end();
    }
    results.offset = offset;
    results.maxsize = uniqueResults.size();
    return results;
}
/**
 *
 */

SearchResults SearchEngine::search_one_index(
    std::string request,
    std::string indexName,
    int size,
    int offset,
    SearchResults& results,
    std::set<int>& uniqueResults
) {
    if (langsDbs.find(indexName) == langsDbs.end()) {
        return SearchResults();
    }
    /* search records */

    int rnum = 0;

    //we first search for words beginning with the
    //request, so that they will appear first
    uint64_t *beginWithResult = tcidbsearch(
        langsDbs[indexName],
        request.c_str(),
        IDBSPREFIX,
        &rnum
    );

    int max = std::min(rnum,size+offset);

    if (beginWithResult) {

        int wordId = 0;
        for (int i = 0; i < rnum; i++) {
            wordId = (int)beginWithResult[i];
            uniqueResults.insert(wordId);
        }
        for (int i = offset; i < max; i++) {
            wordId = (int)beginWithResult[i];
            results.push_back(wordId);
        }
        
        offset = std::max(0, offset- rnum);
        free(beginWithResult);
    } else {
        errorCode = tcidbecode(langsDbs[indexName]);
        std::cerr << "search error: "<< tcidberrmsg(errorCode) << std::endl;
    }

    // and then we make a more generic search
    uint64_t *dystopiaResult = tcidbsearch2(
        langsDbs[indexName],
        request.c_str(),
        &rnum
    );


    if (dystopiaResult) {
        for (
            int i = 0;
            results.size() < size && i < rnum;
            i++
        ) {
            int wordId = (int)dystopiaResult[i];
            // we add it only if it has not been found in the "start with"
            // search ....
            if (uniqueResults.find(wordId) == uniqueResults.end()) {
                if (i >= offset) {
                    results.push_back(wordId);
                }
            } else {
                // otherwise we increment the offset size (as the already match
                // found shoudn't be again in the search result
                // so we compensate that by starting one element after
                if (i < offset) {
                    offset++;
                }
            }
        }

        for (int i = 0; i < rnum; i++) {
            int wordId = (int)dystopiaResult[i];
            uniqueResults.insert(wordId);
        }
        free(dystopiaResult);
    } else {
        errorCode = tcidbecode(langsDbs[indexName]);
        std::cerr << "search error: "<< tcidberrmsg(errorCode) << std::endl;
    }
    return results;
}
