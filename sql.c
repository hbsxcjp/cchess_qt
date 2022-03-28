(iif(length(DATE) == 0, printf('%04d-06-30', substr(EVENT, 1, 4)) , printf('%04d-%02d-%02d', substr(DATE, 1, 4), substr(DATE, 6, instr(DATE, '\xE6\x9C\x88') - 6), iif(instr(DATE, '\xE6\x97\xA5') == 0, 1, substr(DATE, instr(DATE, '\xE6\x9C\x88') + 1, instr(DATE, '\xE6\x97\xA5') - instr(DATE, '\xE6\x9C\x88') - 1) ) ) ) BETWEEN '2000-01-01' AND '2001-01-01')

"''"