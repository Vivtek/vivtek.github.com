create table process (
id text,
owner text,
groupown text,
title text,
description text,
started timestamp,
parent_process text,
parent_task text,
definition text,
datasheet text,
status text
);

create table task (
id text,
process text,
status text,
owner text,
description text,
role text,
queue text,
created timestamp,
sched_date date,
sched_time time,
priority smallint,
complete timestamp,
datasheet text,
subprocess text
);
