CREATE TABLE Events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL, -- FIXME maybe should be an enum like, so INT
    date DATETIME,
    person_id INTEGER REFERENCES People(id),
    place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    comment TEXT
);