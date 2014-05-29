CREATE TABLE Relations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL,
    person1_id INTEGER REFERENCES People(id),
    person2_id INTEGER REFERENCES People(id),
    child_id INTEGER, -- optional
    place TEXT,
    place_id INTEGER DEFAULT 0,
    date TEXT,
    comment TEXT
);
