#include <CTRPluginFramework.hpp>
#include "Item/ItemSearcher.hpp"
#include "Helpers/IDList.hpp"
#include "Helpers/Player.hpp"
#include "Address/Address.hpp"
#include "Item/ItemKeywordChecks.hpp"

namespace CTRPluginFramework {
    ItemCategoryOffset GetItemCategoryOffset(Item_Categories itemCategory) {
		static Address GetItemCategoryLengthAddress(0x5360A8);
		u32 offset = GetItemCategoryLengthAddress.Call<u32>((u8)itemCategory);
		return *(ItemCategoryOffset *)offset;
	}

	/* decomp of 0x769dbc */
	u32 GetShortenedItem(Item item) {
		u32 result = -1;

		if ((item & 0xFFFF7FFF) - 0x2000 < 0x172B) {
			result = (item & 0xFFFF7FFF) - 0x2000;
		}

		return result;
	}

    /* Part decomp of 0x6ba680 */
	u32 GetNextItemByCategory(ItemCategoryOffset offset, u32 index) {
		u32 item = 0x7FFE;
		
		if (index < offset.end) {
			item = offset.begin + index & 0xFFFF;
		}

		return item;
	}

	void ClearItemCategorySizeFromStack(ItemCategoryData *itemCategoryData) {
        for (int i = 0; i < INDEX_LENGTH; ++i) {
            itemCategoryData->index[i] = -1;
        }

        itemCategoryData->category = Item_Categories::Invalid;
    }

	/* decomp of 0x6f02f4 */
	bool WriteItemCategorySizeToStack(ItemCategoryData *itemCategoryData, u32 *UnlockedItems, Item_Categories itemCategory) {
		ClearItemCategorySizeFromStack(itemCategoryData);

        ItemCategoryOffset offset = GetItemCategoryOffset(itemCategory);

		int length = offset.size;
		if (length < INDEX_LENGTH+1) {
			int validIndex = 0;
			if (length != 0) {
                for (int index = 0; index < length; ++index) {
					u32 item = GetNextItemByCategory(offset, index);
					u32 shortenedItem = GetShortenedItem(Item(item));

					bool itemUnlocked = false;
					if ((int)shortenedItem >= 0 && ((shortenedItem >> 5) < 0xBA)) {
						itemUnlocked = (UnlockedItems[shortenedItem >> 5] >> (shortenedItem & 0x1F)) & 1;
					}

					if (itemUnlocked) {
						itemCategoryData->index[validIndex++] = index;
					}
				}
			}

            itemCategoryData->category = itemCategory;
			return true;
		}

		ClearItemCategorySizeFromStack(itemCategoryData);
		return false;
	}

	/* decomp of 0xbe57c4 -> This only gets used for keyword search */
	u32 FUN_BE57C4(u32 *param_1) {
		static Address FUN_00001144(0x2FCC14); //Returns shortened item (?), or u16
		static Address FUN_0000114c(0x2FEA78); //Returns u32(?)
		static Address FUN_00001154(0x2F776C); //Returns void, probably thunk
		
		u32 temp[4];
		u32 *data = FUN_00001144.Call<u32*>(temp, 0x2000);
		u32 res = FUN_0000114c.Call<u32>(param_1, data, 0);
		FUN_00001154.Call<void>(&temp);
		return res;
	}

    /* decomp of 0x771278 */
	u32 GetItemByCategory(ItemCategoryData *itemCategoryData, Item_Categories itemCategory, int index) {
        if (itemCategoryData->category == itemCategory) {
            ItemCategoryOffset offset = GetItemCategoryOffset(itemCategory);

            if (index <= offset.size) {
                return GetNextItemByCategory(offset, itemCategoryData->index[index]);
            }
        }
        return 0x7FFE;
	}

    /* Decomp of 0x7712c0 */
    int GetItemsToSearchCount(ItemCategoryData *itemCategoryData) {
        if (itemCategoryData->category == Item_Categories::Invalid) {
            return 0;
        }
           
        int count = 0;
        ItemCategoryOffset offset = GetItemCategoryOffset(itemCategoryData->category);

        while (itemCategoryData->index[count] != -1 && itemCategoryData->index[count] < offset.size) {
            count++;
        }

        return count;
    }

	/* decomp of 0x2fcc14 */
	void SetItemToStack(u32 *ItemStack, u32 item) {
		ItemStack[0] = item & 0xFFFF7FFF;
		ItemStack[1] = 0;

		u32 uVar2 = ((item & 0xFFFF7FFF) << 0x11) >> 0x1E;
		if (uVar2 < 2) {
			CheckData checkData = SEARCH_CHECKS[uVar2];

			u32 *piVar4 = (u32 *)((u32)ItemStack + (checkData.unk >> 1));
			if ((checkData.unk & 1) == 0) {
				checkData.method(piVar4);
			}
			else {
				OSD::Notify("Error C533: Please report this to the Vapecord Discord Server!", Color::Red);
				//pcVar3 = *(code **)(*piVar4 + *(int *)(SEARCH_CHECKS[uVar2] + iVar1));
			}
		}
	}

	/* decomp of 0x536854 */
	u32 GetItemData(u32 *Item_Stack) {
		u32 item = GetShortenedItem(Item_Stack[0]);
		if ((int)item < 0) {
			return 0;
		}

		u32 addr = *(u32 *)Address(0xA19390).addr;
		if (addr == 0) {
			return 0;
		}

		if (item < 0x172B) {
			return addr + item * 0x1E;
		}

		return 0;
	}

	/* decomp 76a66c */
	/* Don't quite see any pattern when the item is "valid", many categories are valid, many not? */
	u32 IsItemValidCategory(u32 *Item_Stack) {	
		u32 data = GetItemData(Item_Stack);
		if (data != 0) {
			data = ((u32)*(u8 *)(data + 0x1C) << 0x1B) >> 0x1F;
		}
		return data;
	}

	/*
	// decomp of 0x3081e8
	void SetupArray(u32 *array, u32 param_2, u32 param_3) {
		array[0] = &DAT_0090b664;
		array[1] = param_2;
		array[2] = param_3;
		return;
	}

	// decomp of 0x5f56cc
	void SetupArray2(u32 *array) {
		array[0] = 4;
		array[1] = -1;
		array[2] = -1;
		return;
	}

	// decomp of 0x2f83c4
	u32* FUN_002f83c4(u32 *param_1, u32 param_2) {
		u32 uVar1;
		u32 *puVar2;
		u32 *puVar3;

		bool bVar4 = 0x1F < param_2;
		param_2 = param_2 - 0x20;
		do {
			if (bVar4) {
				param_1[0] = 0;
				param_1[1] = 0;
				param_1[2] = 0;
				param_1[3] = 0;
				param_1[4] = 0;
				param_1[5] = 0;
				param_1[6] = 0;
				param_1[7] = 0;
				param_1 = param_1 + 8;
				bVar4 = 0x1F < param_2;
				param_2 = param_2 - 0x20;
			}
		} while (bVar4);

		if ((bool)((u8)(param_2 >> 4) & 1)) {
			*param_1 = 0;
			param_1[1] = 0;
			param_1[2] = 0;
			param_1[3] = 0;
			param_1 = param_1 + 4;
		}

		if ((int)(param_2 << 0x1C) < 0) {
			*param_1 = 0;
			param_1[1] = 0;
			param_1 = param_1 + 2;
		}

		uVar1 = param_2 << 0x1E;
		puVar3 = param_1;
		if ((bool)((u8)((param_2 << 0x1C) >> 0x1E) & 1)) {
			puVar3 = param_1 + 1;
			*param_1 = 0;
		}

		if (uVar1 != 0) {
			puVar2 = puVar3;

			if ((int)uVar1 < 0) {
				puVar2 = (u32 *)((int)puVar3 + 2);
				*(u16 *)puVar3 = 0;
			}

			puVar3 = puVar2;
			if ((uVar1 & 0x40000000) != 0) {
				puVar3 = (u32 *)((int)puVar2 + 1);
				*(u8 *)puVar2 = 0;
			}
			return puVar3;
		}
		return puVar3;
	}

	// decomp of 0x2fea78
	int SetupItemFromItemName(u32 array[], u32 param_2, u32 param_3) {
		int iVar2;
		u16 *puVar3;
		int iVar4;
		
		SetupArray(array, array[24], 0x21);

		array[0] = &PTR_FUN_0090B044;

		SetupArray2(array[3]);

		FUN_002f83c4(array[12], 0x42);

		puVar3 = (u16 *)(array[12]);

		*(u16 *)(array[12]) = DAT_008916F4;

		iVar4 = 0x10;
		do {
			iVar4 = iVar4 + -1;
			puVar3[1] = DAT_008916f4;
			puVar3 = puVar3 + 2;
			*puVar3 = DAT_008916f4;
		} while (iVar4 != 0);

		FUN_0056e93c(iVar2 + -0xC, param_2, param_3);
		return iVar2 + -0xC;
	}
	*/

    void WriteDefaultItemCategoryData(ItemCategoryData *itemCategoryData) {
        itemCategoryData = new ItemCategoryData();

        itemCategoryData->category = Item_Categories::Invalid;
        for (int i = 0; i < INDEX_LENGTH; ++i) {
            itemCategoryData->index[i] = -1;
        }
    }

    void WriteDefaultSearchData(SearchData *searchData) {
        searchData->funcPointer = 0xC0CD54;
        searchData->pointerToSearchString = &searchData->searchString[0];
        searchData->unk_0 = 0x15;
        memset(searchData->searchString, 0, sizeof(searchData->searchString));
        searchData->unk_1[0] = 0;
        searchData->unk_1[1] = 0;
        searchData->unk_1[2] = 1;
        searchData->unk_1[3] = 0;
        searchData->unk_2 = 0x90BA3C;
        searchData->pointerToSearchString2 = &searchData->searchString[0];
        searchData->unk_4 = 0x15;
        searchData->unk_5 = 0;
        searchData->unk_6 = 0;
        searchData->unk_7 = 0;
        searchData->unk_8 = 0;
        searchData->unk_9 = 0;
        WriteDefaultItemCategoryData(&searchData->itemCategoryData);
        searchData->itemCategoryIndex = 0;
        searchData->index = 0;
        searchData->funcPointer2 = (u32)SearchItemByKeywordFUNC;
        searchData->unk_C = 0;
        searchData->unk_D = 0;
    }

    std::vector<u16> foundItems;

    /*
    Catalog open via Catalog Machine makes 21C11C param_1 0x32DC48B8
    Catalog open via Cheat makes 21C11C param_1 0x32dd64e4
    */

    /*
    // This gets called from 0x21CF30
    // decomp of 0x21C11C
    void FUN_21C11C(u32 MenuData) {

        *(u32 *)(Game::BaseInvPointer() + 0xC) is MenuData

        0x32DC48B8 + 0x3BAC is 0x307A6B70 (SearchData)
    }
    */

    /* This method probably writes the data to the search result stack */
    /* decomp of 0xBE6D7C */
    bool FUN_00013d7c(SearchData* searchData) {
        static Address FUN_002F776C(0x2F776C); //Returns void, probably thunk
        static Address FUN_0056E93C(0x56E93C);
        constexpr int MAX_VALUE = 50;

        u32 Item_Stack[21];
        u32 iVar4 = searchData->unk_5;
        int iVar6 = 0;

        u32 uVar7 = searchData->index >> 5;
        if (0 < (int)uVar7) {
            if ((uVar7 & 1) != 0) {
                iVar4 = *(u32 *)(iVar4 + 0xBC0);
            }

            for (int i = searchData->index >> 6; i != 0; --i) {
                iVar4 = *(u32 *)(*(u32 *)(iVar4 + 0xBC0) + 0xBC0);
            }
        }

        if (searchData->index < searchData->unk_8) {
            do {
                if (uVar7 != searchData->index >> 5) {
                    iVar4 = *(int *)(iVar4 + 0xBC0);
                    uVar7 = searchData->index >> 5;
                }

                SetItemToStack(Item_Stack, *(u16 *)(iVar4 + (searchData->index & 0x1F) * 2 + 0xB80));

                FUN_0056E93C.Call<void>(iVar4 + (searchData->index & 0x1F) * 0x5C, Item_Stack, 0);

                FUN_002F776C.Call<void>(Item_Stack);

                ++iVar6;
                ++searchData->index;
                if (MAX_VALUE <= iVar6) {
                    return 0;
                }
            } while ((int)(searchData->index + 1) < searchData->unk_8);
        }
        return 1;
    }

    /* decomp of 0xBE6880 */
    u32 FUN_00013880(SearchData* searchData) {
        u32 uVar1;
        u16 uVar2;
        int iVar5;
        u32 uVar6;
        int iVar7;
        u32 uVar8;
        u16 uVar10;
        int iVar11;
        u16 *puVar12;

        constexpr int MAX_VALUE = 50;
        
        u32 data = 0xA19690;
        iVar11 = 0;
        if (searchData->index < searchData->unk_8 - 1) {
            do {
            uVar8 = searchData->index;
            iVar5 = searchData->unk_5;
            uVar6 = (int)uVar8 >> 5;
            if (0 < (int)uVar6) {
                if ((uVar6 & 1) != 0) {
                    iVar5 = *(int *)(iVar5 + 0xbc0);
                }
                for (iVar7 = (int)uVar8 >> 6; iVar7 != 0; iVar7 = iVar7 - 1) {
                    iVar5 = *(int *)(*(int *)(iVar5 + 0xBC0) + 0xBC0);
                }
            }

            puVar12 = (u16 *)(iVar5 + (uVar8 & 0x1f) * 2 + 0xB80);
            uVar10 = *(u16 *)(data + (u32)*puVar12 * 2 - 0x4000);
            uVar8 = uVar8 + 1;

            if ((int)uVar8 < searchData->unk_8) {
                do {
                    uVar1 = (int)uVar8 >> 5;
                    if (uVar6 != uVar1) {
                        iVar5 = *(int *)(iVar5 + 0xBC0);
                    }

                    if (uVar6 != uVar1) {
                        uVar6 = uVar1;
                    }

                    iVar7 = iVar5 + (uVar8 & 0x1F) * 2;
                    uVar2 = *(u16 *)(iVar7 + 0xB80);

                    if (*(u16 *)(data + (u32)uVar2 * 2 - 0x4000) < uVar10) {
                        uVar10 = *puVar12;
                        *puVar12 = uVar2;
                        *(u16 *)(iVar7 + 0xb80) = uVar10;
                        uVar10 = *(u16 *)(data + (u32)*puVar12 * 2 - 0x4000);
                    }

                    uVar8 = uVar8 + 1;
                } while ((int)uVar8 < searchData->unk_8);
            }

            uVar6 = searchData->index;
            iVar11 = iVar11 + 1;
            searchData->index = uVar6 + 1;

            if (MAX_VALUE <= iVar11) {
                return 0;
            }

            } while ((int)(uVar6 + 1) < searchData->unk_8 - 1);
        }

        searchData->funcPointer2 = (u32)FUN_00013d7c;
        searchData->unk_C = 0xC086A8;
        searchData->index = 0;
        return 0;
    }

    /* This gets called from 0x21C11C(0x32DC48B8) */
    /* decomp of 0xBE6A68 */
	u32 SearchItemByKeywordFUNC(SearchData *searchData/*0x307A6B70*/) {
		static Address FUN_007712C0(0x7712C0); //Returns u32
		static Address FUN_006F02F4(0x6F02F4); //Returns bool
		static Address FUN_002F776C(0x2F776C); //Returns void, probably thunk
		static Address FUN_002FEA78(0x2FEA78); //Returns u32(?)
		static Address FUN_0075C5B8(0x75C5B8); //Returns u32(?)
		static Address FUN_005F56E4(0x5F56E4); //Returns u32, probably thunk
		static Address FUN_00308368(0x308368); //Returns void, probably thunk
		static Address FUN_002FD0BC(0x2FD0BC); //Returns u32 (?)
		static Address FUN_00301B3D(0x301B3D); //Returns u32

		u32 _LAB_00003d58 = 0xC070D0; //Offset 240D0
		u32 _LAB_00003d70 = (u32)FUN_00013880; //Offset 3880

		constexpr int MAX_VALUE = 50; //For what i dont know, seems to stop the search  *(u32 *)(0x849D0C);
		constexpr int MAX_CATEGORIES = sizeof(CATEGORIES_TO_SEARCH) / sizeof(CATEGORIES_TO_SEARCH[0]);
		constexpr int MAX_CHECK_METHODS = sizeof(CHECKS) / sizeof(CHECKS[0]);

		u32 local_b8[3];
		u32 *piVar6;
		u32 iVar11 = 0;

		void* allocated_ptr;

		u32 Item_Stack[21];
		u8 Stack2_AC[80];

		do {
			int i;
		//Set to next category, unsure yet why it is a while loop
			while (i = GetItemsToSearchCount(&searchData->itemCategoryData), i <= searchData->index) {
				++searchData->itemCategoryIndex;

			//If all categories got searched -> exit
				if (MAX_CATEGORIES <= searchData->itemCategoryIndex) {
					searchData->index = 0;
					searchData->funcPointer2 = _LAB_00003d70;
					searchData->unk_C = 0;
					return 0;
				}

				WriteItemCategorySizeToStack(&searchData->itemCategoryData, Player::GetSaveData()->UnlockedItems, CATEGORIES_TO_SEARCH[searchData->itemCategoryIndex]);

				searchData->index = 0;

				iVar11 += 5;
				if (MAX_VALUE <= iVar11) {
					return 0;
				}
			}
			
			u32 item = GetItemByCategory(&searchData->itemCategoryData, CATEGORIES_TO_SEARCH[searchData->itemCategoryIndex], searchData->index);
			SetItemToStack(Item_Stack, item); //goes through every item by category

			u32 res = Address(0x76A66C).Call<u32>(Item_Stack); //IsItemValidCategory(Item_Stack);
			if (res != 0) {
				FUN_002FEA78.Call<u32>(local_b8, Item_Stack, 0); //actually gets the item from the name i believe, stores it in 

				res = FUN_0075C5B8.Call<u32>(local_b8, (u8* )&searchData->unk_2, 1);
            //If keyword search
				if (res == 0) {
					if (searchData->unk_D != 0) {
						int bitShifter = 1;
						int checkIndex = 0;

						do {
							if ((searchData->unk_D & bitShifter) != 0) {
								Keyword_Check_Data checkData = CHECKS[checkIndex];
								if ((checkData.unk & 1U) == 0) {
									res = checkData.method(Item_Stack);
								}

								if (res != 0) {
									goto LAB_00003b84;
								}
							}

							++checkIndex;
							bitShifter <<= 1;
						} while (checkIndex < MAX_CHECK_METHODS);
					}
				}
            //If default item search
				else {
				LAB_00003b84:
                    foundItems.push_back(*Item_Stack & 0x7FFF);
				}
			}

			++iVar11;
			++searchData->index;

			if (MAX_VALUE <= iVar11) {
				return 0;
			}
		} while(true);
	}

	/* This gets called from 0x21C11C(0x32DC48B8) */
    /* decomp of 0xBE6A68 */
	u32 SearchItemByKeywordFUNCTEST(SearchData *searchData/*0x307A6B70*/) {
		static Address FUN_007712C0(0x7712C0); //Returns u32
		static Address FUN_006F02F4(0x6F02F4); //Returns bool
		static Address FUN_002F776C(0x2F776C); //Returns void, probably thunk
		static Address FUN_002FEA78(0x2FEA78); //Returns u32(?)
		static Address FUN_0075C5B8(0x75C5B8); //Returns u32(?)
		static Address FUN_005F56E4(0x5F56E4); //Returns u32, probably thunk
		static Address FUN_00308368(0x308368); //Returns void, probably thunk
		static Address FUN_002FD0BC(0x2FD0BC); //Returns u32 (?)
		static Address FUN_00301B3D(0x301B3D); //Returns u32

		u32 _LAB_00003d58 = 0xC070D0; //Offset 240D0
		u32 _LAB_00003d70 = (u32)FUN_00013880; //Offset 3880

		constexpr int MAX_VALUE = 50; //For what i dont know, seems to stop the search  *(u32 *)(0x849D0C);
		constexpr int MAX_CATEGORIES = sizeof(CATEGORIES_TO_SEARCH) / sizeof(CATEGORIES_TO_SEARCH[0]);
		constexpr int MAX_CHECK_METHODS = sizeof(CHECKS) / sizeof(CHECKS[0]);

		u32 local_b8[3];
		u32 *piVar6;
		u32 iVar11 = 0;

		void* allocated_ptr;

		u32 Item_Stack[21];
		u8 Stack2_AC[80];

		do {
			int i;
		//Set to next category, unsure yet why it is a while loop
			while (i = GetItemsToSearchCount(&searchData->itemCategoryData), i <= searchData->index) {
				++searchData->itemCategoryIndex;

			//If all categories got searched -> exit
				if (MAX_CATEGORIES <= searchData->itemCategoryIndex) {
					searchData->index = 0;
					searchData->funcPointer2 = _LAB_00003d70;
					searchData->unk_C = 0;
					return 0;
				}

				WriteItemCategorySizeToStack(&searchData->itemCategoryData, Player::GetSaveData()->UnlockedItems, CATEGORIES_TO_SEARCH[searchData->itemCategoryIndex]);

				searchData->index = 0;

				iVar11 += 5;
				if (MAX_VALUE <= iVar11) {
					return 0;
				}
			}
			
			u32 item = GetItemByCategory(&searchData->itemCategoryData, CATEGORIES_TO_SEARCH[searchData->itemCategoryIndex], searchData->index);
			SetItemToStack(Item_Stack, item); //goes through every item by category

			u32 res = Address(0x76A66C).Call<u32>(Item_Stack); //IsItemValidCategory(Item_Stack);
			if (res != 0) {
				FUN_002FEA78.Call<u32>(local_b8, Item_Stack, 0); //actually gets the item from the name i believe, stores it in 

				res = FUN_0075C5B8.Call<u32>(local_b8, (u8* )&searchData->unk_2, 1);
            //If keyword search (or its the check below it)
				if (res == 0) {
					if (searchData->unk_D != 0) {
						int bitShifter = 1;
						int checkIndex = 0;

						do {
							if ((searchData->unk_D & bitShifter) != 0) {
								Keyword_Check_Data checkData = CHECKS[checkIndex];
								if ((checkData.unk & 1U) == 0) {
									res = checkData.method(Item_Stack);
								} else {
									OSD::Notify("Error C544: Please report this to the Vapecord Discord Server!", Color::Red);
									//Usually this shouldn't happen, I couldn't find any use of it
									//piVar6 = (u32 *)(param_1 + (checkData.unk >> 1));
									//res = Address(*piVar6 + (u32)checkData.method).Call<u32>(Item_Stack, piVar6);
								}

								if (res != 0) {
									goto LAB_00003b84;
								}
							}

							++checkIndex;
							bitShifter <<= 1;
						} while (checkIndex < MAX_CHECK_METHODS);
					}
				}
            //If default item search
				else {
				LAB_00003b84:
					u16 uVar2 = *Item_Stack & 0x7FFF;

					if (searchData->unk_5 == 0) {
						++searchData->unk_9;

						allocated_ptr = FUN_002FD0BC.Call<void*>(0xBC4, *(u32 *)(0x953CC4), 4); //Malloc
						u32 offset = FUN_00301B3D.Call<u32>(allocated_ptr, (u32)FUN_BE57C4, 0x5C, 0x20);

						*(u32 *)(offset + 0xBC0) = 0;
						searchData->unk_6 = offset;
						searchData->unk_5 = offset;
					}
 
					if (0x1F < searchData->unk_7) {
						searchData->unk_7 = 0;
						++searchData->unk_9;

						allocated_ptr = FUN_002FD0BC.Call<void*>(0xBC4, *(u32 *)(0x953CC4), 4); //Malloc
						u32 offset = FUN_00301B3D.Call<u32>(allocated_ptr, (u32)FUN_BE57C4, 0x5C, 0x20);

						*(u32 *)(offset + 0xBC0) = 0;
						*(u32 *)(searchData->unk_6 + 0xBC0) = offset;
						searchData->unk_6 = offset;
					}

					*(u16 *)(searchData->unk_6 + searchData->unk_7 * 2 + 0xB80) = uVar2;
                    foundItems.push_back(uVar2);
					++searchData->unk_8;
					++searchData->unk_7;
				}

				local_b8[0] = _LAB_00003d58;
				res = FUN_005F56E4.Call<u32>(Stack2_AC);
				FUN_00308368.Call<void>(res + -0xC);
			}

			++iVar11;
			++searchData->index;

			if (MAX_VALUE <= iVar11) {
				FUN_002F776C.Call<void>(Item_Stack);
				return 0;
			}

			FUN_002F776C.Call<void>(Item_Stack);
		} while(true);
	}

	bool Test(std::string& Keyword) {
		SearchData *searchData = new SearchData();
        WriteDefaultSearchData(searchData);

        strncpy(searchData->searchString, Keyword.c_str(), sizeof(searchData->searchString));

		static Address FUN_002FEA78(0x2FEA78); //Returns u32(?)
		u32 Item_Stack[21];
		u32 iVar11 = 0;
		u32 local_b8[3];
		constexpr int MAX_CATEGORIES = sizeof(CATEGORIES_TO_SEARCH) / sizeof(CATEGORIES_TO_SEARCH[0]);
		constexpr int MAX_VALUE = 50; //For what i dont know, seems to stop the search  *(u32 *)(0x849D0C);

		do {
			int i;
		//Set to next category, unsure yet why it is a while loop
			while (i = GetItemsToSearchCount(&searchData->itemCategoryData), i <= searchData->index) {
				++searchData->itemCategoryIndex;

			//If all categories got searched -> exit
				if (MAX_CATEGORIES <= searchData->itemCategoryIndex) {
					searchData->index = 0;
					return 0;
				}

				WriteItemCategorySizeToStack(&searchData->itemCategoryData, Player::GetSaveData()->UnlockedItems, CATEGORIES_TO_SEARCH[searchData->itemCategoryIndex]);

				searchData->index = 0;

				iVar11 += 5;
				if (MAX_VALUE <= iVar11) {
					return 0;
				}
			}

			u32 item = GetItemByCategory(&searchData->itemCategoryData, CATEGORIES_TO_SEARCH[searchData->itemCategoryIndex], searchData->index);
			SetItemToStack(Item_Stack, item); //goes through every item by category

			u32 res = Address(0x76A66C).Call<u32>(Item_Stack); //IsItemValidCategory(Item_Stack);
			if (res != 0) {
				FUN_002FEA78.Call<u32>(local_b8, Item_Stack, 0); //actually gets the item from the name i believe, stores it in 

				OSD::Notify(Utils::Format("Found Item: 0x%04X", *Item_Stack & 0x7FFF), Color::Lime);
			}

			++iVar11;
			++searchData->index;

			if (MAX_VALUE <= iVar11) {
				return 0;
			}
		} while(true);
	}

	bool TestKeyword(std::string& Keyword) {
		Test(Keyword);
		return true;
	}

	bool SearchItemByKeyword(std::string& Keyword) {
        foundItems.clear();

        SearchData *searchData = new SearchData();
        WriteDefaultSearchData(searchData);

        strncpy(searchData->searchString, Keyword.c_str(), sizeof(searchData->searchString));

        SearchItemByKeywordFUNC(searchData);

        OSD::Notify(Utils::Format("%d items found for keyword: %s", foundItems.size(), Keyword.c_str()), Color::Lime);
        OSD::Notify(Utils::Format("Item 1: 0x%04X", foundItems.size() > 0 ? foundItems[0] : 0xFFFF), Color::Lime);

        delete searchData;
        return true;
	}

    /*
    Info about crash of search function:

    0x523D04 -> This Method Crashes, as its parameters are 0

    They should be:
    r0 0x32e0ff98 (Not sure what this is yet) (Comes from 0x95F11C (Which is keyboard pointer for all keyboards))
    r1 0x307a6b7c (This is the SearchData + 0xC alias the Search String itself) (Comes from iVar2 = DAT_0095f108 + DAT_00ad7260;)
    r2 0x0
    

    The crash method usually gets called by 0x5094E8 or actually 0x50ABBC

    This method seems to set 0x95F11C : int sub_524434()

    UPDATE: Fixed the first crash by giving the proper data to it via hook

    Now it crashes at 0x523D6C:
    *(_WORD *)(*(_DWORD *)(param1 + 0x10) + 2 * param3) = 0;

    Fixed that by giving it proper data as well via hook

    Now it crashes at PC 0x28 ?, LR is 0x5094E8:
        v83 = *(_DWORD *)(a1 + 0x1B4);
        0x32e5fa80
    Lets breakpoint it and see what its supposed to be


    r3             0x14                20
    r7             0xad7140            11366720


    r3             0x523d04            5389572
    r7             0xffffb29           268434217

    The Thunk jump table actually goes to 0xBE6A34 (0x13A34 for IDA/Ghidra)

    0xBE6A68 (0x13A68) is our search function

    0x1AF814 is now our next target, it crashes at 0x1af8a8
    It seems to have something to do with listing the items after the search is done
    it takes one parameter, which is the keyboard data? 0x32e0ff98 (Comes from 0x95F11C)

    This might clean the search result data:
    int FUN_0012d2d0(void) (Called by 0x1323A4)

    We want to see who writes 0x32e0ff98 + 0x12cc
    This method writes to it 0x1af868 (Called by 0x55f144)

    This method gives us the search result data to save it to

    FUN_0012e3c0(uVar1 << 2,DAT_00953cc4, 4);
    FUN_001328ec(param_2, param_1, param_3, 1);

    r0             0x32c234f8          851588344
    r1             0x630               1584
    r2             0x4                 4
    r3             0x1                 1

    int __fastcall sub_1328EC(int result, int a2, int a3, int a4) {
        if (!off_976358)
            return sub_2F7460(a2, 0, a3, a4);

        if (result)
            return (*(int (__fastcall **)(int, int, int))(*(_DWORD *)result + 24))(result, a2, a3);

        result = sub_3047C8(off_976358);
        if (result)
            return (*(int (__fastcall **)(int, int, int))(*(_DWORD *)result + 24))(result, a2, a3);

        return result;
    }


    0x330bcf10
    
    */

    /* decomp of 0xBE6A34 */
    u32 FUN_00013A34(SearchData *searchData) {
        u32 funcToCall = searchData->funcPointer2;
        if ((searchData->unk_C & 1) != 0) {
            //Lets hope this doesn't happen :D
            OSD::Notify("Error C888: Please report this to the Vapecord Discord Server!", Color::Red);
            //funcToCall = *(code **)(funcToCall + *(int *)(param_1 + ((int)searchData->unk_C >> 1)));
        }

        return Address(funcToCall).Call<u32>(searchData);
    }
}