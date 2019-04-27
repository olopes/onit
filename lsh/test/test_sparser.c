/* prod code includes */
#include "test_definitions.h"
#include "sparser.h"
#include "cstr_sparser_adapter.h"
#include "wcstr_sparser_adapter.h"
#include "stdio_sparser_adapter.h"
#include "assert_sexpr.h"

UnitTest(sparse_should_do_parse_string_and_return_ok) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    wchar_t * data = L"\"HEY!\"";
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    
    expected_object = sexpr_create_cstring(L"HEY!");
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_do_parse_symbol_and_return_ok) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    wchar_t * data = L" HEY!\n ";
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_object = sexpr_create_csymbol(L"HEY!");
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_do_parse_list_and_return_ok) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    wchar_t * data = L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)\n";
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_object = sexpr_cons(sexpr_create_csymbol(L"hey"), 
                            sexpr_cons(sexpr_create_csymbol(L"y-o-u"),
                                sexpr_cons(sexpr_create_cstring(L"check them dubs"),
                                    sexpr_cons(sexpr_create_csymbol(L"123"),NULL))));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_do_parse_pair_and_return_ok) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    char * data = "(left . right)";
    
    stream = create_sparser_stream(CSTR_ADAPTER, data, strlen(data));
    expected_object = sexpr_cons(sexpr_create_csymbol(L"left"), sexpr_create_csymbol(L"right"));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_parse_a_list_from_a_file) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    char * filename = "test/sexpression.lsh"; /* (abc |d.e-f| "ghi" 1/3+4i)  */
    
    
    stream = create_sparser_stream(FILE_NAME_ADAPTER, filename);
    expected_object = sexpr_cons(sexpr_create_csymbol(L"abc"), 
                            sexpr_cons(sexpr_create_csymbol(L"d.e-f"),
                                sexpr_cons(sexpr_create_cstring(L"ghi"),
                                    sexpr_cons(sexpr_create_csymbol(L"1/3+4i"),NULL))));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_parse_a_list_from_a_file_descriptor) {
    struct sexpression * expected_object;
    struct sexpression * actual_object;
    struct sparser_stream * stream;
    char * filename = "test/sexpression.lsh"; /* (abc |d.e-f| "ghi" 1/3+4i)  */
    FILE * input;
    
    input = fopen(filename, "rb");
    if(input == NULL) {
        print_error("Could not open the test file '%s'\n", filename);
        skip();
        return;
    }
    
    stream = create_sparser_stream( FILE_DESCRIPTOR_ADAPTER, input);
    expected_object = sexpr_cons(sexpr_create_csymbol(L"abc"), 
                            sexpr_cons(sexpr_create_csymbol(L"d.e-f"),
                                sexpr_cons(sexpr_create_cstring(L"ghi"),
                                    sexpr_cons(sexpr_create_csymbol(L"1/3+4i"),NULL))));
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);
    
    release_sparser_stream(stream);
    
    fclose(input);
}

UnitTest(sparse_should_parse_multiple_objects_from_a_stream) {
    struct sexpression * actual_object;
    struct sexpression * expected_objects[4];
    struct sexpression ** expected_object;
    struct sparser_stream * stream;
    wchar_t * data = L"symbol\"glued string\"\n(hey |y-o-u| 123)      ";
    
    /* wchar_t * data = L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)"; */
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_objects[0] = sexpr_create_csymbol(L"symbol");
    expected_objects[1] = sexpr_create_cstring(L"glued string");
    expected_objects[2] = sexpr_cons(sexpr_create_csymbol(L"hey"), 
                            sexpr_cons(sexpr_create_csymbol(L"y-o-u"),
                                sexpr_cons(sexpr_create_csymbol(L"123"),NULL)));
    expected_objects[3] = NULL;
    
    expected_object = expected_objects;
    while(*expected_object) {
        assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
        assert_sexpr_equal(*expected_object, actual_object);
        sexpr_free(actual_object);
        sexpr_free(*expected_object);
        expected_object++;
    }
    
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_EOF);
    assert_null(actual_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_ignore_comments) {
    struct sexpression * actual_object;
    struct sexpression * expected_object;
    struct sparser_stream * stream;
    wchar_t * data = L"; single line comment\n#|multi\nline\ncomment|#\n2";
    
    /* wchar_t * data = L"(hey   |y-o-u|    \"check them dubs\" \n 123\n)"; */
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    expected_object = sexpr_create_csymbol(L"2");
    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    assert_sexpr_equal(expected_object, actual_object);
    sexpr_free(actual_object);
    sexpr_free(expected_object);

    
    assert_int_equal(sparse(stream, &actual_object), SPARSE_EOF);
    assert_null(actual_object);
    
    release_sparser_stream(stream);
}

UnitTest(sparse_should_return_pair_with_quote_symbol_when_data_starts_with_single_quote) {
    struct sexpression * actual_object;
    struct sexpression * expected_object;
    struct sparser_stream * stream;
    wchar_t * data = L"'symbol";
    
    expected_object = sexpr_cons(sexpr_create_csymbol(L"quote"), sexpr_cons(sexpr_create_csymbol(L"symbol"), NULL));
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);
    
    sexpr_free(actual_object);
    sexpr_free(expected_object);

    release_sparser_stream(stream);
}

UnitTest(sparse_should_return_value_with_quoted_chars_when_data_is_string_with_quoted_chars) {
    struct sexpression * actual_object;
    struct sexpression * expected_object;
    struct sparser_stream * stream;
    wchar_t * data = L"\" \\n \\r \\\" \\' \\\\ \\ua \"";
    
    expected_object = sexpr_create_cstring(L" \n \r \" ' \\ \n ");
    
    stream = create_sparser_stream(WCSTR_ADAPTER, data,wcslen(data));
    assert_int_equal(sparse(stream, &actual_object), SPARSE_OK);
    
    assert_sexpr_equal(expected_object, actual_object);

    sexpr_free(actual_object);
    sexpr_free(expected_object);

    release_sparser_stream(stream);
}

