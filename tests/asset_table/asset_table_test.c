#include "../../include/assert_manager.h"
#include "check.h"
START_TEST(tc_insert_then_validate_count)
{
  AssetTable tbl;

  asset_table_init(&tbl);

  asset_table_insert(&tbl, "key1", "value1");
  asset_table_insert(&tbl, "key2", "value2");
  asset_table_insert(&tbl, "key3", "value3");
  asset_table_insert(&tbl, "key4", "value4");
  asset_table_insert(&tbl, "key5", "value5");
  asset_table_insert(&tbl, "key6", "value6");
  asset_table_insert(&tbl, "key7", "value7");
  asset_table_insert(&tbl, "key8", "value8");
  asset_table_insert(&tbl, "key7", "value9");
  asset_table_insert(&tbl, "key10", "value10");
  asset_table_insert(&tbl, "key11", "value11");
  asset_table_insert(&tbl, "key12", "value12");
  asset_table_insert(&tbl, "key13", "value13");
  asset_table_insert(&tbl, "key14", "value14");
  asset_table_insert(&tbl, "key15", "value15");
  asset_table_insert(&tbl, "key16", "value16");
  asset_table_insert(&tbl, "key17", "value17");
  asset_table_insert(&tbl, "key18", "value18");
  asset_table_insert(&tbl, "key2", "new value 2");
  asset_table_insert(&tbl, "key11", "new value 11");
  asset_table_insert(&tbl, "key10", "new value 10");

  ck_assert_uint_eq(tbl.usedCnt, 18);

  asset_table_erase(&tbl, "key5");

  ck_assert_uint_eq(tbl.usedCnt, 17);

  asset_table_destroy(&tbl);
}
END_TEST

START_TEST(tc_insert_then_lookup)
{
  AssetTable tbl;

  asset_table_init(&tbl);

  asset_table_insert(&tbl, "key1", "value1");
  asset_table_insert(&tbl, "key2", "value2");
  asset_table_insert(&tbl, "key3", "value3");
  asset_table_insert(&tbl, "key4", "value4");
  asset_table_insert(&tbl, "key5", "value5");
  asset_table_insert(&tbl, "key6", "value6");
  asset_table_insert(&tbl, "key7", "value7");
  asset_table_insert(&tbl, "key8", "value8");
  asset_table_insert(&tbl, "key7", "value9");
  asset_table_insert(&tbl, "key10", "value10");
  asset_table_insert(&tbl, "key11", "value11");
  asset_table_insert(&tbl, "key12", "value12");
  asset_table_insert(&tbl, "key13", "value13");
  asset_table_insert(&tbl, "key14", "value14");
  asset_table_insert(&tbl, "key15", "value15");
  asset_table_insert(&tbl, "key16", "value16");
  asset_table_insert(&tbl, "key17", "value17");
  asset_table_insert(&tbl, "key18", "value18");
  asset_table_insert(&tbl, "key2", "new value 2");
  asset_table_insert(&tbl, "key11", "new value 11");
  asset_table_insert(&tbl, "key10", "new value 10");

  ck_assert_str_eq(asset_table_lookup(&tbl, "key5"), "value5");
  ck_assert_str_eq(asset_table_lookup(&tbl, "key10"), "new value 10");
  ck_assert_str_eq(asset_table_lookup(&tbl, "key11"), "new value 11");

  asset_table_destroy(&tbl);
}
END_TEST

START_TEST(tc_insert_then_erase_then_lookup)
{
  AssetTable tbl;

  asset_table_init(&tbl);

  asset_table_insert(&tbl, "key1", "value1");
  asset_table_insert(&tbl, "key2", "value2");
  asset_table_insert(&tbl, "key3", "value3");
  asset_table_insert(&tbl, "key4", "value4");
  asset_table_insert(&tbl, "key5", "value5");
  asset_table_insert(&tbl, "key6", "value6");
  asset_table_insert(&tbl, "key7", "value7");
  asset_table_insert(&tbl, "key8", "value8");
  asset_table_insert(&tbl, "key7", "value9");
  asset_table_insert(&tbl, "key10", "value10");
  asset_table_insert(&tbl, "key11", "value11");
  asset_table_insert(&tbl, "key12", "value12");
  asset_table_insert(&tbl, "key13", "value13");
  asset_table_insert(&tbl, "key14", "value14");
  asset_table_insert(&tbl, "key15", "value15");
  asset_table_insert(&tbl, "key16", "value16");
  asset_table_insert(&tbl, "key17", "value17");
  asset_table_insert(&tbl, "key18", "value18");
  asset_table_insert(&tbl, "key2", "new value 2");
  asset_table_insert(&tbl, "key11", "new value 11");
  asset_table_insert(&tbl, "key10", "new value 10");

  asset_table_erase(&tbl, "key6");
  ck_assert_ptr_eq(asset_table_lookup(&tbl, "key6"), NULL);

  asset_table_erase(&tbl, "key2");
  ck_assert_ptr_eq(asset_table_lookup(&tbl, "key6"), NULL);
  asset_table_destroy(&tbl);
}
END_TEST

Suite*
create_asset_table_test_suite(void)
{
  Suite* suite = suite_create("asset_table_test_suite");

  TCase* tcInsertThenValiateCount;
  TCase* tcInsertThenLookup;
  TCase* tcEraseThenLookup;

  tcInsertThenValiateCount = tcase_create("tc_insert_then_validate_count");
  tcase_add_test(tcInsertThenValiateCount, tc_insert_then_validate_count);

  tcInsertThenLookup = tcase_create("tc_insert_then_lookup");
  tcase_add_test(tcInsertThenLookup, tc_insert_then_lookup);

  tcEraseThenLookup = tcase_create("tc_insert_then_erase_then_lookup");
  tcase_add_test(tcEraseThenLookup, tc_insert_then_erase_then_lookup);

  suite_add_tcase(suite, tcInsertThenValiateCount);
  suite_add_tcase(suite, tcInsertThenLookup);
  suite_add_tcase(suite, tcEraseThenLookup);
  return suite;
}

int
main(int argc, char** argv)
{
  SRunner* runner = srunner_create(create_asset_table_test_suite());

  srunner_run_all(runner, CK_VERBOSE);
  int numberFailed = srunner_ntests_failed(runner);
  srunner_free(runner);
  return numberFailed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
