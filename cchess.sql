--update project set sort_id = id + 10000 where id < 155;
--update project set sort_id = company_id where id > 155;

--update employee set department_id = 1, position_id0 = 4, position_id1 = null, role_id = 3;

--SELECT CURRENT_TIMESTAMP;

--CREATE TABLE [project_1](
--  [id] INTEGER PRIMARY KEY AUTOINCREMENT,
--  [old_id] INTEGER,
--  [company_id] INTEGER REFERENCES [company]([id]), 
--  [proName] TEXT);

--INSERT INTO project_1 (old_id, company_id, proName) 
--   SELECT id, company_id, proName
--   FROM project
--   ORDER BY sort_id;


--update employee set joinDate = 0, atWork = 1;

--select * from project where end_date IS NULL AND company_id IN (3);

--select * from project where company_id IN (2) AND end_date IS NULL;

--select * from employee where project_id IN (2) AND start_date IS NOT NULL;

--update employee_history set start_date = date('2021-11-23'), end_date = '2021-11-28';

--SELECT strftime('%s', '2021-11-23');
--select date('now');

--update company set sort_id = id;

--select *, length(regstr) as len from ecco where length(regstr) > 0 order by sn desc;

--select count(*) from manual;

--select * from manual where id=9506;
--select count(*) from manual where length(rowcols) < 8;
--select count(*) from manual where eccosn = '';
--SELECT count(*) FROM manual WHERE CALUATE_ECCOSN != ECCOSN;

--SELECT count(*) FROM manual WHERE length(DATE_OLD) == 0;
--SELECT count(*) FROM manual WHERE length(DATE) == 10;
--select strftime('%s', '2000-01-01');
--select printf('%02d', 2);

--update manual set DATE = printf('%04d-06-30', iif(instr(EVENT, '年') == 0, 2001, substr(EVENT, 1, instr(EVENT, '年') - 1)))
--                          WHERE length(date_old) = 0;
--update manual set DATE = printf('%04d-%02d-%02d', substr(DATE_OLD, 1,4), substr(DATE_OLD, 6,instr(DATE_OLD, '月')-6),
--      iif(instr(DATE_OLD, '日')==0, 1, 
--                          substr(DATE_OLD, instr(DATE_OLD, '月')+1, instr(DATE_OLD, '日')-instr(DATE_OLD, '月')-1)));


select * from manual where length(DATE) == 0;

select strftime(iif(length(DATE) == 0, printf('%04d-06-30', iif(instr(EVENT, '\xE5\xB9\xB4') == 0, 2001, substr(EVENT, 1, instr(EVENT, '\xE5\xB9\xB4') - 1))) , printf('%04d-%02d-%02d', substr(DATE, 1, 4), substr(DATE, 6, instr(DATE, '\xE6\x9C\x88') - 6), iif(instr(DATE, '\xE6\x97\xA5') == 0, 1, substr(DATE, instr(DATE, '\xE6\x9C\x88') + 1, instr(DATE, '\xE6\x97\xA5') - instr(DATE, '\xE6\x9C\x88') - 1))) )) >= strftime('1970-01-01') AND strftime(iif(length(DATE) == 0, printf('%04d-06-30', iif(instr(EVENT, '\xE5\xB9\xB4') == 0, 2001, substr(EVENT, 1, instr(EVENT, '\xE5\xB9\xB4') - 1))) , printf('%04d-%02d-%02d', substr(DATE, 1, 4), substr(DATE, 6, instr(DATE, '\xE6\x9C\x88') - 6), iif(instr(DATE, '\xE6\x97\xA5') == 0, 1, substr(DATE, instr(DATE, '\xE6\x9C\x88') + 1, instr(DATE, '\xE6\x97\xA5') - instr(DATE, '\xE6\x9C\x88') - 1))) )) <= strftime('2030-01-01') AND 1 AND 1 AND 1 AND 1 AND 1 AND 1 AND 1
 from manual where length(DATE) == 0;
--SELECT * FROM manual WHERE title like '%?%';

--SELECT count(*) FROM manual WHERE date(DATE) > date('1980-12-31') and date(DATE) < date('2020-12-31');


--select count(*) from manual  
--where strftime(DATE) >= strftime('1970-01-01') AND strftime(DATE) <= strftime('2000-01-01') AND 1 AND 1 AND 1 AND 1 AND 1 AND 1;