#Generated by aw-scanner

ifeq ($(AW_CONVERTER),)
AW_CONVERTER:=aw-converter
endif

AW_SRCS:=tests/page1_html.c tests/page2_html.c tests/nobody_png.c tests/sub_page1_html.c tests/sub_page2_html.c tests/sub_index_html.c tests/index_html.c 

tests/page1_html.c: tests/www/page1.html.aw tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c page1_html  $<

tests/page2_html.c: tests/www/page2.html.aw tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c page2_html  $<

tests/nobody_png.c: tests/www/nobody.png tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c nobody_png -b $<

tests/sub_page1_html.c: tests/www/sub/page1.html.aw tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c sub_page1_html  $<

tests/sub_page2_html.c: tests/www/sub/page2.html.aw tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c sub_page2_html  $<

tests/sub_index_html.c: tests/www/sub/index.html.aw tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c sub_index_html  $<

tests/index_html.c: tests/www/index.html.aw tests/server_test.mk
	$(AW_CONVERTER) -o $@ -c index_html  $<

