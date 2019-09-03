SECTION_DATA_PROLOGUE(test_int_sections,,)
{
    __test_int_start = .;
    KEEP(*(".test_int"));
    __test_int_end = .;
} GROUP_LINK_IN(ROMABLE_REGION)
