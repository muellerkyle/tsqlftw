var db = require('./tsqlftw').tsqlftwObject;
var mssql = new db();

mssql.Connect('server=localhost\\sqlexpress;Trusted_Connection=yes;database=TestDB;', function (err, data) {
  mssql.Query('select * from myTable', function (err, results) {
    console.log(err);
    console.log(results);
    results = JSON.parse(results);
    console.log(results.error);
    console.log(results.count);
    // result set is in results.rows and has a row number.
    mssql.Close(function (err, results) {
      console.log('Connection closed.');
    });
  });
});

