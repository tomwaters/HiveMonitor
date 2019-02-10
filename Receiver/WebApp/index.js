const express = require('express');
var bodyParser = require("body-parser");
const app = express();
var mysql = require('mysql');
var ps = require('ps-node');
var config = require('./config');

var sqlPool = mysql.createPool({
    host: config.dbIP,
    database: config.dbName,
    user: config.dbUser,
    password: config.dbPassword,
    timezone: 'utc',
    multipleStatements: true
});

function sqlQuery(query, success) {
    sqlPool.getConnection(function(err, connection) {
        if(err) {
            console.log(err);
        } else {
            connection.query(query, function(error, results, fields) {
                if(error) {
                    console.log(error);
                } else {
                    connection.release();
                    success(results);
                }
            });
        }
    });
}

app.use('/', express.static(__dirname + '/static'));
app.use('/js', express.static(__dirname + '/node_modules/bootstrap/dist/js')); // redirect bootstrap JS
app.use('/js', express.static(__dirname + '/node_modules/jquery/dist')); // redirect JS jQuery
app.use('/js', express.static(__dirname + '/node_modules/moment/min')); // redirect moment
app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css')); // redirect CSS bootstrap

app.use('/js', express.static(__dirname + '/node_modules/pdfmake/build')); // redirect pdfmake
app.use('/js', express.static(__dirname + '/node_modules/jszip/dist')); // redirect jszip
app.use('/js', express.static(__dirname + '/node_modules/datatables.net/js')); // redirect datatables
app.use('/js', express.static(__dirname + '/node_modules/datatables.net-bs4/js')); // redirect datatables
app.use('/js', express.static(__dirname + '/node_modules/datatables.net-buttons/js')); // redirect datatables buttons
app.use('/js', express.static(__dirname + '/node_modules/datatables.net-buttons-bs4/js')); // redirect datatables bootstrap4 buttons
app.use('/css', express.static(__dirname + '/node_modules/datatables.net-bs4/css')); // redirect CSS datatables
app.use('/css', express.static(__dirname + '/node_modules/datatables.net-buttons-bs4/css')); // redirect CSS datatables bootstrap4 buttons

app.use('/js', express.static(__dirname + '/node_modules/chart.js/dist')); // redirect datatables bootstrap4 buttons
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

app.get('/api/latest', function(req, res) {
    sqlQuery('SELECT * FROM LatestData', function(results) {
        res.json(results);
    });
});

app.get('/api/logs', function(req, res) {
    var num = parseInt(req.query.length);
    var start = parseInt(req.query.start);
    var order = 'ORDER BY ';
    req.query.order.forEach(function(element) {
        order += req.query.columns[parseInt(element.column)].data + ' ' + element.dir + ' ';
    });
    var query = 'SELECT COUNT(ID) AS num_records FROM data_received;';
    query += 'SELECT * FROM Logs ' + order + 'LIMIT ' + start + ', ' + num +';';

    sqlQuery(query, function(results) {
        res.json({
            draw: parseInt(req.query.draw),
            recordsTotal: results[0][0].num_records,
            recordsFiltered: results[0][0].num_records,
            data: results[1]
        });
    });
});

app.post('/api/logs/delete', function(req, res) {
    var query = 'DELETE FROM data_received WHERE ID IN(';
    query += req.body.rowIds.map(function(e) {return parseInt(e);}).join(',');
    query += ');';
    sqlQuery(query, function(results) {
        res.json({success: true});
    });
});

app.get('/api/last6months', function(req, res) {
    sqlQuery('SELECT * FROM Last6Months ORDER BY Time DESC', function(results) {
        res.json(results);
    });
});

app.get('/api/last24hours', function(req, res) {
    sqlQuery('SELECT * FROM Last24Hours ORDER BY Time DESC', function(results) {
        res.json(results);
    });
});

app.get('/api/last7days', function(req, res) {
    sqlQuery('SELECT * FROM Last7Days ORDER BY Time DESC', function(results) {
        res.json(results);
    });
});

app.get('/api/status', function(req, res) {
    ps.lookup({
        command: 'bee_receiver'
    }, function(err, resultList) {
        var receiver_running = !err;
        res.json({receiver_running: receiver_running});
    });
});

app.listen(80, () => console.log('Hive app listening on port 80!'));
