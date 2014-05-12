CREATE TABLE People (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sex VARCHAR(1), -- FIXME maybe should be an enum like, so INT; one of M/F/U
    first_name TEXT,
    middle_name TEXT,
    surname TEXT,
    prefix TEXT,
    suffix TEXT,
    honorary TEXT,
    birth_date DATETIME,
    birth_place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    christening_date DATETIME,
    christening_place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    death_date DATETIME NULL,
    death_place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    death_reason TEXT,
    burial_date DATETIME,
    burial_place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    occupation TEXT,
    occupation_place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    contact_email TEXT,
    contact_phone TEXT,
    contact_web TEXT,
    religion TEXT,
    nationality TEXT,
    height INTEGER, -- in cm
    weight INTEGER, -- in kg
    color TEXT,
    eye_color TEXT,
    hair_color TEXT,
    comment TEXT,
    photo BLOB
);
