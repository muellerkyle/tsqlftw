using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Data.SqlClient;
using System.Data;
using Newtonsoft.Json;

namespace tsqlftw
{
    public class tsqlftwClass
    {

        public SqlConnection Conn;

        public bool Connect(string connString)
        {
            try
            {
                Conn = new SqlConnection(connString);
                Conn.Open();
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public string Query(string query)
        {
            var output = new Dictionary<string,object>();
            var recordSet = new List<Dictionary<string, object>>();

            try
            {
                var cmd = new SqlCommand(query, Conn);
                var res = cmd.ExecuteReader();
                var count = 0;
                while (res.Read())
                {
                    count++;
                    var x = new Dictionary<string, object>();
                    for (var i = 0; i < res.FieldCount; i++)
                    {
                        x[res.GetName(i)] = res.GetValue(i);
                    }
                    recordSet.Add(x);
                }
				res.Close();
                output["error"] = "";
                output["count"] = count;
                output["rows"] = recordSet;
            }
            catch (Exception ex)
            {
                output.Clear();
                output["error"] = "An exception occured.";
                output["count"] = -1;
            }

            return JsonConvert.SerializeObject(output);

        }

        public bool Close()
        {
            try
            {
                Conn.Close();
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public tsqlftwClass()
        {

        }
    }
}
