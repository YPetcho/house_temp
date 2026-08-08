// Fill these with your values before building
#ifndef SECRETS_H
#define SECRETS_H

// WiFi
#define WIFI_SSID "Yonatan and Michal"
#define WIFI_PASS "26099955"

// Supabase
// Example: https://abcd1234efgh.supabase.co
#define SUPABASE_URL "https://zkkrdccwxwqipaykqmad.supabase.co"
// Use your anon or service_role key. For inserts from a device, anon key + RLS policies are preferred.
#define SUPABASE_KEY "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Inpra3JkY2N3eHdxaXBheWtxbWFkIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTc4NjEwNTM1NiwiZXhwIjoyMTAxNjgxMzU2fQ.MPJPeyCZBZWv6Pcu3cnNt7MdoV8R26o0sD0MwHlvwGA"
// The table to insert into (e.g. temperatures)
#define SUPABASE_TABLE "temp_logs"

// DHT data pin. Keep it on D4 because that is where the sensor is wired.
#define DHT_PIN D4

#endif // SECRETS_H
