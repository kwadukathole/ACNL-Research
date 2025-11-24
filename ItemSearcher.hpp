#ifndef ITEMSEARCHER_HPP
#define ITEMSEARCHER_HPP

#include <CTRPluginFramework.hpp>
#include "Helpers/GameStructs.hpp"
#include "Address/Address.hpp"

namespace CTRPluginFramework {
	using CheckFunc = void(*)(u32*);

    inline constexpr const Item_Categories CATEGORIES_TO_SEARCH[15] = {
        Wallpaper, Carpets, Furniture, 
        Shirts, Trousers, Dresses, 
		Hats, Accesories, Shoes, 
        Socks, Umbrellas, MailPapers, 
        Songs, Gyroids, AnalyzedFossils
    };

	struct __attribute__((packed)) ItemCategoryOffset {
		u16 begin;
		u16 end;
		u32 size;
	};

	struct __attribute__((packed)) ItemCategoryData {
		Item_Categories category;
		u32 index[0x6ED];
	};

    inline constexpr int INDEX_LENGTH = sizeof(ItemCategoryData::index) / sizeof(ItemCategoryData::index[0]);

    /*
    I have no idea why the SearchString is pointed to twice in the struct, but it is
    */
	struct __attribute__((packed)) SearchData {
		/*0x0000*/ u32 funcPointer; //Always 0xC0CD54
        /*0x0004*/ char* pointerToSearchString;
        /*0x0008*/ u32 unk_0; //0x15?
        /*0x000C*/ char searchString[40];
        /*0x0034*/ u8 unk_1[4]; //0 1 0 0
		/*0x0038*/ u32 unk_2; //Always 0x90BA3C
		/*0x003C*/ char* pointerToSearchString2;
		/*0x0040*/ u32 unk_4;
		/*0x0044*/ u32 unk_5; //Pointer to data
		/*0x0048*/ u32 unk_6; //Pointer to data
		/*0x004C*/ u32 unk_7;
		/*0x0050*/ u32 unk_8;
		/*0x0054*/ u32 unk_9;
		/*0x0058*/ ItemCategoryData itemCategoryData;
		/*0x1C10*/ int itemCategoryIndex;
		/*0x1C14*/ int index; //Found Items
		/*0x1C18*/ u32 funcPointer2; //Is pointer to search method and 2 other methods, but while searching its the search method 0xBE6A68
		/*0x1C1C*/ u32 unk_C;
		/*0x1C20*/ u32 unk_D;
	};

	struct __attribute__((packed)) CheckData {
		CheckFunc method;
		u32 unk;
	};

	inline void SetItemToStruct(u32* Struct) {
		static Address FUN_006BBBF0(0x6BBBF0);
		return FUN_006BBBF0.Call<void>(Struct);
	}

	inline void SetItemToStruct2(u32* Struct) {
		static Address FUN_006BBBF0(0x6BBAE4);
		return FUN_006BBBF0.Call<void>(Struct);
	}

	inline constexpr CheckData SEARCH_CHECKS[2] = {
		{ SetItemToStruct, 0 }, 
		{ SetItemToStruct2, 0 }
	};

    bool SearchItemByKeyword(std::string& Keyword);
    u32 SearchItemByKeywordFUNC(SearchData *searchData/*0x307A6B70*/);
    void WriteDefaultSearchData(SearchData *searchData);
    u32 FUN_00013A34(SearchData *searchData);

	bool TestKeyword(std::string& Keyword);
}
#endif