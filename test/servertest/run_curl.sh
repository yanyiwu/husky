CURL_RES=curl.res
TMP=curl.res.tmp
curl -s "http://127.0.0.1:11257" >> $TMP
diff $TMP $CURL_RES

rm $TMP
