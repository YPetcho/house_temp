# ESP32 DHT11 → Supabase logger

Place your WiFi and Supabase settings in `include/secrets.h` before building.

Table schema suggestion for Supabase (SQL):

CREATE TABLE temperatures (
  id uuid DEFAULT uuid_generate_v4() PRIMARY KEY,
  temperature numeric,
  device text,
  inserted_at timestamptz DEFAULT now()
);

Build and upload with PlatformIO in the `house_temp` project.
