# To-do manager, take 1.
# Copyright (c) 2000, Vivtek.
# Released under the terms of the GNU license.

set todomgr_home /usr/local/AOLserver/vivtek/pages/todomgr/
set todomgr_root /todomgr
set todomgr_pool wftk
ns_register_proc GET  $todomgr_root/create todomgr_create
ns_register_proc POST $todomgr_root/create todomgr_create
ns_register_proc GET  $todomgr_root/start todomgr_start
ns_register_proc POST $todomgr_root/start todomgr_start
ns_register_proc GET  $todomgr_root/show todomgr_show
ns_register_proc POST $todomgr_root/show todomgr_show
ns_register_proc GET  $todomgr_root/complete todomgr_complete
ns_register_proc POST $todomgr_root/complete todomgr_complete
ns_register_proc GET  $todomgr_root/reject todomgr_reject
ns_register_proc POST $todomgr_root/reject todomgr_reject
ns_register_proc GET  $todomgr_root/update todomgr_update
ns_register_proc POST $todomgr_root/update todomgr_update

ns_register_proc GET  $todomgr_root/overview todomgr_overview
proc todomgr_create {conn ignore} {
global todomgr_pool
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set form [ns_conn form $conn]
if {$form == ""} {
set row [ns_db select $db "select * from process"]
set tags(processlist) "<option value=\"\">Select a process if applicable\n"
while {[ns_db getrow $db $row]} {
   append tags(processlist) "<option value=\"[ns_set get $row id]\">[ns_set get $row title]\n"
}
todomgr_pageout $conn fresh_task.html
  return
}
if {[ns_set get $form what] == "task"} {
  if ![string compare [ns_set get $form description] ""] {
set process [ns_set get $form process]
if ![string compare "" $process] {
   return [todomgr_pageout $conn newtasknoproc.html]
} 
set tags(process) $process
set prow [ns_db select $db "select * from process where id='[sql_safe_string $process]'"]
if ![ns_db getrow $db $prow] {
   set tags(title) "Can't create task for nonexistent process"
   set tags(body) "The process code you entered (<code>$process</code>) can't be found in the database."
   return [todomgr_pageout $conn message.html]
}
set tags(title) [ns_set get $prow title]
if [string compare $user [ns_set get $prow owner]] {
   set perm 0
   set row [ns_db select $db "select * from keyword, permission where keyword.process='[sql_safe_string $process]' and keyword.keyword=permission.keyword and permission.userid='[sql_safe_string $user]'"]
   while {[ns_db getrow $db $row]} {
      if [string match *t* [ns_set get $row flags]] { set perm 1 }
   }
   if !$perm {
      set tags(body) "You don't have sufficient privileges to request tasks for <i>$tags(title)</i>"
      set tags(title) "Insufficent privilege"
      return [todomgr_pageout $conn message.html]
   }
}
set    query "select * from keyword, permission, users "
append query   "where keyword.process='[sql_safe_string $process]' "
append query     "and keyword.keyword=permission.keyword and permission.userid=users.userid "
append query     "and users.permlevel > 1"
append query " order by name"

set row [ns_db select $db $query]
set tags(userlist) "<option value=\"$user\">Select a requestee here\n"
while {[ns_db getrow $db $row]} {
   append tags(userlist) "<option value=\"[ns_set get $row userid]\">"
   append tags(userlist) "[ns_set get $row name] ([ns_set get $row userid])\n"
}
 
todomgr_pageout $conn newtask.html
  } else {
set task $conn[now tag]
set fields [list id status created]
set values [list '$task' 'active' '[now]']
foreach field {process description priority sched_date sched_time} {
   if {[ns_set find $form $field] == -1} { continue }
   if {[ns_set get $form $field] == ""} { continue }

   lappend fields $field
   if {$field == "priority"} {
      lappend values [ns_set get $form $field]
   } else {
      lappend values "'[sql_safe_string [ns_set get $form $field]]'"
   }
}

if {[lsearch $fields owner] == -1} {
   lappend fields owner
   lappend values "'[sql_safe_string $user]'"
}

set query "insert into task ("
append query [join $fields ", "]
append query ") values ("
append query [join $values ", "]
append query ")"

ns_db dml $db $query

ns_returnredirect $conn show?task=$task
  }
} elseif {[ns_set get $form what] == "process"} {
  if ![string compare [ns_set get $form title] ""] {
return [todomgr_pageout $conn newprocess.html]
  } else {
set process $conn[now tag]
set fields [list id status started]
set values [list '$process' 'pending' '[now]']
foreach field {title description} {
   if {[ns_set find $form $field] == -1} { continue }
   if {[ns_set get $form $field] == ""} { continue }

   lappend fields $field
   lappend values "'[sql_safe_string [ns_set get $form $field]]'"
}

if {[lsearch $fields owner] == -1} {
   lappend fields owner
   lappend values "'[sql_safe_string $user]'"
}

set query "insert into process ("
append query [join $fields ", "]
append query ") values ("
append query [join $values ", "]
append query ")"

ns_db dml $db $query

ns_returnredirect $conn show?process=$process
  }
} else {
  set tags(title) "Invalid create request"
  set tags(body) "
To create a task, use <code>what=task</code>; for a process use <code>what=process</code>.
You entered <code>what=[ns_set get $form what]</code>."

  todomgr_pageout $conn message.html
}
}

proc todomgr_start {conn ignore} {
global todomgr_pool
global todomgr_home
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set form [ns_conn form $conn]
if {$form == ""} { set form [ns_set create] }

if [string compare "" [ns_set get $form procdef]] {
set tags(procdef) [ns_set get $form procdef]
set pipe [open "|$todomgr_home/pdm starter $tags(procdef)" "r"]
set tags(title) [gets $pipe]
set tags(version) [gets $pipe]
set tags(list) [read $pipe]
close $pipe

return [todomgr_pageout $conn start.html]
}
if [string compare "" [ns_set get $form start]] {
set process $conn[now tag]
global todomgr_datasheets
set pipe [open "|${todomgr_home}pdm datasheet [ns_set get $form start] [ns_set get $form ver] $process" "r"]
set datasheet [read $pipe]
close $pipe

set pipe [open "$todomgr_datasheets/$process" "w"]
puts $pipe $datasheet
close $pipe

set size [ns_set size $form]
for {set i 0} {$i < $size} {incr i} {
   if {-1 < [lsearch {start ver} [ns_set key $form $i]]} { continue }
   datasheet_setvalue $process "" [ns_set key $form $i] "" [ns_set value $form $i]
}

set pipe [open "|${todomgr_home}wftk start $process" "r"]
set workflow [read $pipe]
close $pipe

set wf [split $workflow \n]

set fields [list id status started]
set values [list '$process' 'active' '[now]']

lappend fields title
set title [string range [lindex $wf 0] 2 end]
lappend values "'[sql_safe_string $title]'"

set wf [lrange $wf 2 end]
while {![string compare "" [lindex $wf 0]]} { set wf [lrange $wf 1 end] }

set desc ""
while {[string compare EOF [lindex $wf 0]]} {
   append desc "[lindex $wf 0]<br>"
   set wf [lrange $wf 1 end]
}
set wf [lrange $wf 1 end]

lappend fields description
lappend values "'[sql_safe_string $desc]'"

lappend fields owner
lappend values "'[sql_safe_string $user]'"

lappend fields definition
set procdef "[ns_set get $form start]_[ns_set get $form ver].xml"
lappend values "'[sql_safe_string $procdef]'"

lappend fields datasheet
lappend values "'$process'"

set query "insert into process ("
append query [join $fields ", "]
append query ") values ("
append query [join $values ", "]
append query ")"

ns_db dml $db $query

wftk_interpret $db $process $wf

return [ns_returnredirect $conn overview?process=$process]
}

set pipe [open "|$todomgr_home/pdm list start?procdef=%s" "r"]
set tags(list) [read $pipe]
close $pipe

return [todomgr_pageout $conn startlist.html]
}

proc todomgr_show {conn ignore} {
set form [ns_conn form $conn]
if {$form == ""} {
   set tags(title) "Nothing to show"
   set tags(body) "You need to select something to show."
   return [todomgr_pageout $conn message.html]
}
global todomgr_pool
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set back [ns_set get $form back]
if [string compare $back ""] {
   set tags(back) "<input type=checkbox checked name=back value=\"$back\">"
   append tags(back) "Return to list after update"
} else {
   set tags(back) ""
}
append back "[ns_conn url $conn]?[ns_conn query $conn]"
append tags(back) "<input type=hidden name=back value=\"$back\">\n"
set task [ns_set get $form task]
if {$task != ""} {
if [catch {set row [ns_db select $db "select * from task where id='$task'"]} result] {
   set tags(title) "Task $task unknown"
   set tags(body) "The task ID <code>$task</code> is not in the database.  Sorry."
   return [todomgr_pageout $conn message.html]
}

if [ns_db getrow $db $row] {
   foreach field {process status priority owner description created sched_date sched_time} {
      set tags($field) [ns_set get $row $field]
   }
   set tags(task) $task
   if [string compare "edit" [ns_set get $form mode]] {
set process [ns_set get $row process]
set tags(processid) $process
if {$process != ""} {
   if [catch {set prow [ns_db select $db "select * from process where id='$process'"]} result] {
      set tags(process) "<i>Unknown process $process</i>"
   } elseif [ns_db getrow $db $prow] {
      set p [ns_set get $prow title]
      set tags(process) "<a href=\"overview?process=$process&who=all\">"
      append tags(process) "[ns_set get $prow title]</a>"
   } else {
      set tags(process) "<i>Unknown process $process</i>"
   }
} else {
   set    query "select id, title from process where owner='[sql_safe_string $user]' "
   append query "union "
   append query "select id, title from process, keyword, permission "
   append query   "where process.id=keyword.process and keyword.keyword=permission.keyword "
   append query   "  and permission.userid='[sql_safe_string $user]' "
   append query   "  and flags like '%t%'"
   append query " order by title"
   if [catch {set prow [ns_db select $db $query]} result] {
      set tags(process) "<i>Unable to generate process list</i>"
   } else {
      set tags(process) "<select name=process>\n"
      append tags(process) "<option value=\"\">Select a process to assign\n"
      set lastid ""
      while {[ns_db getrow $db $prow]} {
         if ![string compare $lastid [ns_set get $prow id]] { continue }
         set lastid [ns_set get $prow id]
         append tags(process) "<option value=\"[ns_set get $prow id]\">"
         append tags(process) "[ns_set get $prow title]\n"
      }
      append tags(process) "</select>\n"
   }
}
set owner $tags(owner) 
if {$owner != ""} {
   if [catch {set urow [ns_db select $db "select * from users where userid='$owner'"]} result] {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   } elseif [ns_db getrow $db $urow] {
      if [string compare "" [ns_set get $urow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $urow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $urow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $urow name]] {
         append tags(ownertag) " ([ns_set get $urow name])"
      }
   } else {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   }
}
      set tags(updatelink) ""
      set tags(isowner) 0
      if ![string compare $tags(owner) $user] {
         set tags(isowner) 1
         set tags(updatelink) "show?task=$task&back=$back&mode=edit"
         set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
         set tags(taskdata) [datasheet_showdata edit $tags(backhere) $tags(processid) $task]
      }
      return [todomgr_pageout $conn taskhome.html]
   } else {
set tags(prioritybox) "<select name=\"priority\">"
set p [ns_set get $row priority]
if {$p == ""} {
   append tags(prioritybox) "<option value=\"\" selected>No priority selected\n"
   append tags(prioritybox) "<option value=1>1\n"
   append tags(prioritybox) "<option value=2>2\n"
   append tags(prioritybox) "<option value=3>3\n"
} else {
   for {set i 1} {$i < $p} {incr i} {
      append tags(prioritybox) "<option value=$i>$i\n"
   }
   append tags(prioritybox) "<option value=$p selected>$p\n"
   incr p
   append tags(prioritybox) "<option value=$p selected>$p\n"
   incr p
   append tags(prioritybox) "<option value=$p selected>$p\n"
}
append tags(prioritybox) "</select>\n"
set process [ns_set get $row process]
set tags(processid) $process
if {$process != ""} {
   if [catch {set prow [ns_db select $db "select * from process where id='$process'"]} result] {
      set tags(process) "<i>Unknown process $process</i>"
   } elseif [ns_db getrow $db $prow] {
      set p [ns_set get $prow title]
      set tags(process) "<a href=\"overview?process=$process&who=all\">"
      append tags(process) "[ns_set get $prow title]</a>"
   } else {
      set tags(process) "<i>Unknown process $process</i>"
   }
} else {
   set    query "select id, title from process where owner='[sql_safe_string $user]' "
   append query "union "
   append query "select id, title from process, keyword, permission "
   append query   "where process.id=keyword.process and keyword.keyword=permission.keyword "
   append query   "  and permission.userid='[sql_safe_string $user]' "
   append query   "  and flags like '%t%'"
   append query " order by title"
   if [catch {set prow [ns_db select $db $query]} result] {
      set tags(process) "<i>Unable to generate process list</i>"
   } else {
      set tags(process) "<select name=process>\n"
      append tags(process) "<option value=\"\">Select a process to assign\n"
      set lastid ""
      while {[ns_db getrow $db $prow]} {
         if ![string compare $lastid [ns_set get $prow id]] { continue }
         set lastid [ns_set get $prow id]
         append tags(process) "<option value=\"[ns_set get $prow id]\">"
         append tags(process) "[ns_set get $prow title]\n"
      }
      append tags(process) "</select>\n"
   }
}
set owner $tags(owner) 
if {$owner != ""} {
   if [catch {set urow [ns_db select $db "select * from users where userid='$owner'"]} result] {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   } elseif [ns_db getrow $db $urow] {
      if [string compare "" [ns_set get $urow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $urow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $urow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $urow name]] {
         append tags(ownertag) " ([ns_set get $urow name])"
      }
   } else {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   }
}
      set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
      set tags(taskdata) [datasheet_showdata view $tags(backhere) $tags(processid) $task]
      return [todomgr_pageout $conn task.html]
   }
} else {
   set tags(title) "Task $task unknown"
   set tags(body) "The task ID <code>$task</code> is not in the database.  Sorry."
   return [todomgr_pageout $conn message.html]
}
   return
}
set process [ns_set get $form process]
if {$process != ""} {
if {![string compare $process all] || ![string compare $process mine]} {
set tags(header) "<tr bgcolor=cccccc><td>Process</td>"
if ![string compare $process all] {
   set tags(title) "All processes"
   append tags(header) "<td>Owner</td>"
   set    query "select id, title, owner, status, text 'prmt' as flags from process "
   append query   "where owner='[sql_safe_string $user]' "
   append query "union "
   append query "select id, title, owner, status, flags from process, keyword, permission "
   append query   "where process.id=keyword.process and keyword.keyword=permission.keyword "
   append query     "and permission.userid='[sql_safe_string $user]' "
   append query "order by title"
} else {
   set tags(title) "My processes"
   set    query "select id, title, owner, status, text 'prmt' as flags from process "
   append query   "where owner='[sql_safe_string $user]' "
   append query "union "
   append query "select id, title, owner, status, flags from process, keyword, permission "
   append query   "where process.id=keyword.process and keyword.keyword=permission.keyword "
   append query     "and permission.userid='[sql_safe_string $user]'"
   append query     "and flags like '%p%' "
   append query "order by title"
}
append tags(header) "<td>Status</td></tr>"

set tags(table) ""
set rows 0
set row [ns_db select $db $query]
set lastid ""
while {[ns_db getrow $db $row]} {
   if ![string compare $lastid [ns_set get $row id]] { continue }
   set lastid [ns_set get $row id]

   append tags(table) "<tr bgcolor=\""
   if [expr $rows % 2] {
      append tags(table) eeeeee
   } else {
      append tags(table) ffffff
   }
   append tags(table) "\">"
   incr rows

   append tags(table) "<td><a href=\"overview?process=[ns_set get $row id]&who=all\">"
   append tags(table) "[ns_set get $row title]</a> "
   if [string match *p* [ns_set get $row flags]] {
      append tags(table) "(<a href=\"show?process=[ns_set get $row id]\">modify</a>)</td>"
   }

   if [string compare $process mine] {
      append tags(table) "<td>[ns_set get $row owner]</td>"
   }

   append tags(table) "<td>[ns_set get $row status]</td></tr>\n"
}
if {$rows == 0} {
   set tags(body) "<tr><td colspan=3><i>No processes found</i></td></tr>"
}

return [todomgr_pageout $conn processlist.html]
}
if [catch {set row [ns_db select $db "select * from process where id='$process'"]} result] {
   set tags(title) "Process $process unknown"
   set tags(body) "The process ID <code>$process</code> is not in the database.  Sorry."
   return [todomgr_pageout $conn message.html]
}

if [ns_db getrow $db $row] {
   foreach field {title owner description started} {
      set tags($field) [ns_set get $row $field]
   }
   set tags(process) $process
set owner $tags(owner) 
if {$owner != ""} {
   if [catch {set urow [ns_db select $db "select * from users where userid='$owner'"]} result] {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   } elseif [ns_db getrow $db $urow] {
      if [string compare "" [ns_set get $urow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $urow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $urow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $urow name]] {
         append tags(ownertag) " ([ns_set get $urow name])"
      }
   } else {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   }
}
set query "select * from permission where userid='[sql_safe_string $user]'"
set mgtwords [list]
if ![catch {set row [ns_db select $db $query]} result] {
   while {[ns_db getrow $db $row]} {
      set keyword([ns_set get $row keyword]) off
      if [string match *p* [ns_set get $row flags]] { lappend mgtwords [ns_set get $row keyword] }
   }
}
set query "select * from keyword where keyword.process='[sql_safe_string $process]'"
if ![catch {set row [ns_db select $db $query]} result] {
   if {[string tolower $user] == [string tolower $tags(owner)]} {
      while {[ns_db getrow $db $row]} {
         set keyword([ns_set get $row keyword]) on
      }
   } else {
      while {[ns_db getrow $db $row]} {
         if [info exists keyword([ns_set get $row keyword])] {
            set keyword([ns_set get $row keyword]) on
         }
      }
   }
}
set tags(keywordlist) ""
foreach word [lsort [array names keyword]] {
   append tags(keywordlist) "<input type=checkbox name=\"keyword\" value=\"$word\""
   if {$keyword($word) == "on"} { append tags(keywordlist) " checked" }
   append tags(keywordlist) ">"
   if {[lsearch $mgtwords $word] > -1} {
      append tags(keywordlist) "<a href=\"keywords?keyword=$word\">$word</a>"
   } else {
      append tags(keywordlist) $word
   }
   append tags(keywordlist) "<br>\n"
}
   set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
   set tags(processdata) [datasheet_showdata edit $tags(backhere) $process ""]
   return [todomgr_pageout $conn process.html]
} else {
   set tags(title) "Process $process unknown"
   set tags(body) "The process ID <code>$process</code> is not in the database.  Sorry."
   return [todomgr_pageout $conn message.html]
}
   return
}

set tags(title) "Nothing to show"
set tags(body) "You need to select something to show."
todomgr_pageout $conn message.html
}

proc todomgr_complete {conn ignore} {
global todomgr_pool
global todomgr_home
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set form [ns_conn form $conn]
set task ""
if {$form != ""} { set task [ns_set get $form task] }
if {$task == ""} {
   set tags(title) "No task specified"
   set tags(body) "Task completion requires a task identifier."
   return [todomgr_pageout $conn message.html]
}
set query "select * from task where id='[sql_safe_string $task]'"
if {[catch {set row [ns_db select $db $query]} result] || ![ns_db getrow $db $row]} {
   set tags(title) "Task unknown"
   set tags(body) "The task you specified (<code>$task</code>) could not be found in the database."
   return [todomgr_pageout $conn message.html]
}

if [string compare $user [ns_set get $row owner]] {
   set tags(title) "Insufficient privilege"
   set tags(body) "You are not the owner of this task.  Only the owner may complete a task."
   return [todomgr_pageout $conn message.html]
}
set process [ns_set get $row process]
if {$process == ""} {
   set query "update task set status='complete',complete='[now]'"
   append query " where id='[sql_safe_string $task]'"
   ns_db dml $db $query
   if [string compare [ns_set get $form back] ""] {
      return [ns_returnredirect $conn [ns_set get $form back]]
   }
   set tags(title) "Task complete"
   set tags(body) "The task has been marked as complete"
   return [todomgr_pageout $conn message.html]
}
set query "select * from process where id='[sql_safe_string $process]'"
set row [ns_db select $db $query]
if ![ns_db getrow $db $row] {
   set query "update task set status='complete', complete='[now]'"
   append query " where id='[sql_safe_string $task]'"
   ns_db dml $db $query
   if [string compare [ns_set get $form back] ""] {
      return [ns_returnredirect $conn [ns_set get $form back]]
   }
   set tags(title) "Task complete"
   set tags(body) "The task has been marked as complete"
   return [todomgr_pageout $conn message.html]
}
if [string compare "" [ns_set get $row definition]] {
   set pipe [open "|${todomgr_home}wftk complete $process $task" "r"]
   set wf [split [read $pipe] \n]
   close $pipe
   wftk_interpret $db $process $wf
}
set query "update task set status='complete',complete='[now]' where id='[sql_safe_string $task]'"
ns_db dml $db $query
set query "select count(*) as ct from task where status<>'complete' and process='[sql_safe_string $process]'"
set row [ns_db select $db $query]
ns_db getrow $db $row
if {[ns_set get $row ct] == 0} {
   set query "update process set status='complete' where id='[sql_safe_string $process]'"
   ns_db dml $db $query
}

if [string compare [ns_set get $form back] ""] {
   return [ns_returnredirect $conn [ns_set get $form back]]
}
return [ns_returnredirect $conn "overview?process=$process"]
}

proc todomgr_reject {conn ignore} {
global todomgr_pool
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set form [ns_conn form $conn]
set task ""
if {$form != ""} { set task [ns_set get $form task] }
if {$task == ""} {
   set tags(title) "No task specified"
   set tags(body) "Task rejection requires a task identifier."
   return [todomgr_pageout $conn message.html]
}
set query "select * from task where id='[sql_safe_string $task]'"
if {[catch {set row [ns_db select $db $query]} result] || ![ns_db getrow $db $row]} {
   set tags(title) "Task unknown"
   set tags(body) "The task you specified (<code>$task</code>) could not be found in the database."
   return [todomgr_pageout $conn message.html]
}

if [string compare $user [ns_set get $row owner]] {
   set tags(title) "Insufficient privilege"
   set tags(body) "You are not the owner of this task.  Only the owner may reject a task."
   return [todomgr_pageout $conn message.html]
}
set process [ns_set get $row process]
if {$process == ""} {
   set query "update task set status='rejected'"
   append query " where id='[sql_safe_string $task]'"
   ns_db dml $db $query
   if [string compare [ns_set get $form back] ""] {
      return [ns_returnredirect $conn [ns_set get $form back]]
   }
   set tags(title) "Task rejected"
   set tags(body) "The task has been marked as rejected"
   return [todomgr_pageout $conn message.html]
}
set query "select * from process where id='[sql_safe_string $process]'"
set row [ns_db select $db $query]
if ![ns_db getrow $db $row] {
   set query "update task set status='rejected'"
   append query " where id='[sql_safe_string $task]'"
   ns_db dml $db $query
   if [string compare [ns_set get $form back] ""] {
      return [ns_returnredirect $conn [ns_set get $form back]]
   }
   set tags(title) "Task rejected"
   set tags(body) "The task has been marked as rejected"
   return [todomgr_pageout $conn message.html]
}
if [string compare "" [ns_set get $row definition]] {
   set tags(title) "wftk must handle rejection"
   set tags(body) "This task must be rejected by the wftk engine."
   return [todomgr_pageout $conn message.html]
}
set query "update task set status='rejected' where id='[sql_safe_string $task]'"
ns_db dml $db $query

if [string compare [ns_set get $form back] ""] {
   return [ns_returnredirect $conn [ns_set get $form back]]
}
set tags(title) "Task complete"
set tags(body) "The task has been marked as complete"
return [todomgr_pageout $conn message.html]
}

proc todomgr_update {conn ignore} {
set form [ns_conn form $conn]
if {$form == ""} {
   set tags(title) "Nothing to update"
   set tags(body) "You need to specify something to update."
   return [todomgr_pageout $conn message.html]
}
global todomgr_pool
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}

set task [ns_set get $form task]
if {$task != ""} {
set query "select * from task where id='[sql_safe_string $task]'"
set row [ns_db select $db $query]
ns_db getrow $db $row
set modify 1
set priority 1
if [string compare $user [ns_set get $row owner]] {
   set modify 0
   set priority 0
   set process [ns_set get $row process]
   if [string compare $process ""] {
      set query "select * from process where id='[sql_safe_string $process]'"
      set row [ns_db select $db $query]
      ns_db getrow $db $row
      set modify 1
      set priority 1
      if [string compare $user [ns_set get $row owner]] {
         set modify 0
         set priority 0
         set    query "select flags from keyword, permission "
         append query   "where keyword.process='[sql_safe_string $process]' "
         append query     "and keyword.keyword=permission.keyword "
         append query     "and permission.userid='[sql_safe_string $user]'"
         set row [ns_db select $db $query]
         while {[ns_db getrow $db $row]} {
            if [string match *m* [ns_set get $row flags]] { set modify 1 }
            if [string match *r* [ns_set get $row flags]] { set priority 1 }
         }
      }
   }
}
if {!$modify && !$priority} {
   set tags(title) "Insufficient privilege"
   set tags(body) "You don't have sufficient privilege to update this task."
   return [todomgr_pageout $conn message.html]
}
set fields [list]
foreach field {process description priority sched_date sched_time} {
   if {[ns_set find $form $field] == -1} { continue }
   if {[ns_set get $form $field] == ""} { continue }
   if {!$modify && $field != "priority"} { continue }

   if {$field == "priority"} {
      lappend fields "$field=[ns_set get $form $field]"
   } else {
      lappend fields "$field='[sql_safe_string [ns_set get $form $field]]'"
   }
}

if {[llength $fields] > 0} {
   set query "update task set "
   append query [join $fields ", "]
   append query " where id='$task'"

   ns_db dml $db $query
}
set back [ns_set get $form back]
if [string compare $back ""] {
   ns_returnredirect  $conn $back
} else {
   set tags(title) "Update complete"
   set tags(body) "Your update operation was completed."
   todomgr_pageout $conn message.html
}
return
}
set process [ns_set get $form process]
if {$process != ""} {
set fields [list]
foreach field {title description} {
   if {[ns_set find $form $field] == -1} { continue }
   if {[ns_set get $form $field] == ""} { continue }

   lappend fields "$field='[sql_safe_string [ns_set get $form $field]]'"
}

if {[llength $fields] > 0} {
   set query "update process set "
   append query [join $fields ", "]
   append query " where id='$process'"

   ns_db dml $db $query
}
set back [ns_set get $form back]
if [string compare $back ""] {
   ns_returnredirect  $conn $back
} else {
   set tags(title) "Update complete"
   set tags(body) "Your update operation was completed."
   todomgr_pageout $conn message.html
}
return
}

set tags(title) "Nothing to update"
set tags(body) "You need to specify either a task or a process to update."
todomgr_pageout $conn message.html
}

proc todomgr_overview {conn ignore} {
global todomgr_pool
set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}

set form [ns_conn form $conn]
if {$form == ""} {
   set form [ns_set create]
   set filter active
   set order priority
} else {
   set filter [ns_set get $form filter]

   set order [ns_set get $form order]
   if ![string compare $order ""] {
      switch -- $filter {
         active { set order priority }
         scheduled { set order schedule }
         complete { set order process }
         full { set order process }
         default { set order priority }
      }
   }
}
set process [ns_set get $form process]
if ![string compare $filter ""] {
   if [string compare $process ""] {
      set filter full
   } else {
      set filter active
   }
}
switch -- $filter {
   active    { set where "and task.status='active'" }
   scheduled { set where "and not sched_date is null" }
   complete  { set where "and task.status='complete'" }
   full      { set where "" }
   default   { set where "and task.status='active'" }
}

switch -- $order {
   priority { set orderby "order by priority desc, created" }
   schedule { set orderby "order by sched_date, sched_time, created" }
   process  { set orderby "order by title, created" }
   default  { set orderby "order by created" }
}

switch -- $filter {
   active    { set tags(title) "To do" }
   scheduled { set tags(title) "To do on date" }
   complete  { set tags(title) "Completed tasks" }
   full      { set tags(title) "All tasks" }
   default   { set tags(title) "To do" }
}
set who [ns_set get $form who]
if ![string compare $who ""] { set who $user }
if [string compare $who all] {
   append where " and task.owner='[sql_safe_string $who]'"
}
if [string compare $process ""] {
   set prow [ns_db select $db "select * from process where id='[sql_safe_string $process]'"]
   if ![ns_db getrow $db $prow] {
      set tags(title) "Process not found"
      set tags(body) "The process <code>$process</code> couldn't be found in the database."
      return [todomgr_pageout $conn message.html]
   }
   foreach field {title description owner} { set tags($field) [ns_set get $prow $field] }

set owner $tags(owner) 
if {$owner != ""} {
   if [catch {set urow [ns_db select $db "select * from users where userid='$owner'"]} result] {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   } elseif [ns_db getrow $db $urow] {
      if [string compare "" [ns_set get $urow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $urow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $urow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $urow name]] {
         append tags(ownertag) " ([ns_set get $urow name])"
      }
   } else {
      set tags(ownertag) "<i>Unknown owner $owner</i>"
   }
}

   if ![string compare $tags(owner) $user] {
      set tags(modlink) "show?process=$process"
      append tags(modlink) "&back=[ns_urlencode [ns_conn url $conn]?[ns_conn query $conn]]"
      set tags(requestlink) "create?what=task&process=$process"
      append tags(requestlink) "&back=[ns_urlencode [ns_conn url $conn]?[ns_conn query $conn]]"
   } else {
      set    query "select flags from keyword, permission "
      append query   "where keyword.process='[sql_safe_string $process]' "
      append query     "and keyword.keyword=permission.keyword "
      append query     "and permission.userid='[sql_safe_string $user]'"
      set tags(requestlink) ""
      if ![catch {set row [ns_db select $db $query]} result] {
         while {[ns_db getrow $db $row]} {
            if [string match *t* [ns_set get $row flags]] {
               set tags(requestlink) "create?what=task&process=$process"
               append tags(requestlink) "&back=[ns_urlencode [ns_conn url $conn]?[ns_conn query $conn]]"
            }
         }
      }
   }

   append where " and task.process='[sql_safe_string $process]'"
   set tags(process) $process
}
set query "
select task.*, text '' as title from task where process is null $where
union
select task.*, process.title as title from task, process where task.process=process.id $where
$orderby"
set tags(header) "<tr bgcolor=\"cccccc\"><td>Task</td><td>Owner</td>"
set cols 3
if ![string compare $process ""] {
   append tags(header) "<td>Process</td>"
   incr cols
}
if {$order == "priority"} {
   append tags(header) "<td>Priority</td>"
   incr cols
}

if {$filter == "scheduled"} {
   append tags(header) "<td>Scheduled</td>"
} else {
   append tags(header) "<td>Created</td>"
}

if {$filter == "full"} {
   append tags(header) "<td>Status</td>"
}

append tags(header) "</tr>\n"
set back "[ns_conn url $conn]"
if [string compare "" [ns_conn query $conn]] { append back "?[ns_conn query $conn]" }
set back [ns_urlencode $back]
set rows 0
set tags(table) ""
if ![catch {set row [ns_db select $db $query]} result] {
   while {[ns_db getrow $db $row]} {
      append tags(table) "<tr bgcolor=\""
      if [expr $rows % 2] {
         append tags(table) eeeeee
      } else {
         append tags(table) ffffff
      }
      append tags(table) "\">"
      append tags(table) "<td><a href=\"show?task=[ns_set get $row id]&back=$back\">"
      if [string compare "" [ns_set get $row description]] {
         append tags(table) [ns_set get $row description]
      } else {
         append tags(table) [ns_set get $row id]
      }
      append tags(table) "</a></td>"
      append tags(table) "<td>[ns_set get $row owner]</td>"
      if ![string compare $process ""] {
         if {[ns_set get $row process] != ""} {
            append tags(table) "<td>"
            append tags(table) "<a href=\"overview?process=[ns_set get $row process]&who=all&filter=active\">"
            append tags(table) "[ns_set get $row title]</a></td>"
         } else {
            append tags(table) "<td>&nbsp;</td>"
         }
      }
      if {$order == "priority"} {
         set p [ns_set get $row priority]
         if {$p == ""} { set p 1 }
         append tags(table) "<td><center>"
         append tags(table) "<a href=update?task=[ns_set get $row id]"
         append tags(table) "&priority=[expr $p + 1]&back=$back>up</a> "
         append tags(table) "$p "
         if {$p < 2} {
            append tags(table) "dn"
         } else {
            append tags(table) "<a href=update?task=[ns_set get $row id]"
            append tags(table) "&priority=[expr $p - 1]&back=$back>dn</a> "
         }
         append tags(table) "</center></td>"
      }
      if {$filter == "scheduled"} {
         append tags(table) "<td>[ns_set get $row sched_date] [ns_set get $row sched_time]</td>"
      } else {
         append tags(table) "<td>[ns_set get $row created]</td>"
      }
      if {$filter == "full"} {
         append tags(table) "<td><i>[ns_set get $row status]</i></td>"
      }
      append tags(table) "</tr>\n"
      incr rows
   }
}

if {$rows == 0} {
   set tags(table) "<tr><td colspan=$cols><i>No tasks found</i></td></tr>\n"
}

if [string compare $process ""] {
   set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
   set tags(processdata) [datasheet_showdata view "" $process ""]
   todomgr_pageout $conn processhome.html
} else {
   todomgr_pageout $conn overview.html
}
}
proc todomgr_pageout {conn file {status 200}} {
   upvar tags tags
   global todomgr_home
   set fn $todomgr_home/$file
   if {![file exists $fn]} {
   return [ns_return $conn 404 text/html "
<h1>404</h1>
<hr>
  <blockquote>
  <i>You step in the stream<br>
     the water has moved on<br>
     page not found
  </i>
  </blockquote>

  Sorry, <code>[ns_conn url $conn]</code> can't be found.
"]
   }

   set fil [open $fn]
   while {[gets $fil line] >= 0} {
      set hit [regexp -nocase {\[##([-a-z_ 0-9!/?]*)##\]} $line match tag]
      while {$hit} {
         regsub -all -nocase \\\[##$tag##\\\] $line [escape_ampersand [todomgr_pageout_tag_value $tag]] line
         set hit [regexp -nocase {\[##([-a-z_ 0-9!/?]*)##\]} $line match tag]
      }
      append pg $line "\n"
   }
   close $fil

   ns_return $conn $status text/html $pg
}

proc escape_ampersand {str} {
   regsub -all "&" $str \\\\\\& retval
   return $retval
}
proc todomgr_pageout_tag_value {tag} {
   upvar tags tags
   if [string match "flagopen *" $tag] {
      set tag [string range $tag 9 end]
      if [info exists tags($tag)] {
         if {$tags($tag) != 0 && $tags($tag) != ""} { return "" }
      }
      return "<!--"
   }
   if [string match "flagclose *" $tag] {
      set tag [string range $tag 10 end]
      if [info exists tags($tag)] {
         if {$tags($tag) != 0 && $tags($tag) != ""} { return "" }
      }
      return "-->"
   }
   if [info exists tags($tag)] { return $tags($tag) }
   return ""
} 
proc now {{what all}} {
   set time [ns_localtime]
   switch $what {
    all {return [format "%04d-%02d-%02d %02d:%02d:%02d" [expr [lindex $time 5] + 1900] [expr [lindex $time 4] + 1] [lindex $time 3] [lindex $time 2] [lindex $time 1] [lindex $time 0]]}
    date {return [format "%04d-%02d-%02d" [expr [lindex $time 5] + 1900] [expr [lindex $time 4] + 1] [lindex $time 3]]}
    time {return [format "%02d:%02d:%02d" [lindex $time 2] [lindex $time 1] [lindex $time 0]]}
    tag {return [format "%04d%02d%02d%02d%02d%02d" [expr [lindex $time 5] + 1900] [expr [lindex $time 4] + 1] [lindex $time 3] [lindex $time 2] [lindex $time 1] [lindex $time 0]]}
   }
}

proc sql_safe_string {s} {
   set out ""
   while {[string first "'" $s] > -1} {
      set f [string first "'" $s]
      append out [string range $s 0 $f]
      append out "'"
      set s [string range $s [expr $f + 1] end]
   }
   append out $s

   return $out
}
ns_register_proc GET $todomgr_root/user todomgr_user_admin
ns_register_proc POST $todomgr_root/user todomgr_user_admin
proc todomgr_user_admin {conn ignore} {
   set form [ns_conn form $conn]
   if {$form == ""} {
      set action list
   } else {
      set action [ns_set get $form action]
   }

   global todomgr_pool
   set db [ns_db gethandle $todomgr_pool]

   switch -- $action {
      list {
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
if {[ns_set get $userrow permlevel] < 3} {
   set tags(title) "Insufficient privileges"
   set tags(body) "Your permission level is lower than 3; a level of 3 is required to view the list of users.  Sorry."
   return [todomgr_pageout $conn message.html]
}

set row [ns_db select $db "select * from users"]
set tags(body) ""
while {[ns_db getrow $db $row]} {
   append tags(body) "<tr><td>"
   append tags(body) "<input type=checkbox name=delete value=\""
   append tags(body) "[ns_urlencode [ns_set get $row userid]]\">"
   append tags(body) "<a href=user?action=modify&user=[ns_urlencode [ns_set get $row userid]]>"
   append tags(body) "[ns_set get $row name] ([ns_set get $row userid])</a></td>"
   append tags(body) "</td></tr>\n"
}
if {$tags(body) == ""} {
   append tags(body) "<i>No users found</i>"
}
todomgr_pageout $conn user_list.html
         return
      }
      add {
set fields [list]
set values [list]
set userfields {userid password name email website}

foreach field $userfields {
   set value [ns_set get $form $field]
   set tags($field) $value
   if {$value == ""} { continue }
   lappend fields $field
   if {$field != "permlevel"} {
      lappend values "'[sql_safe_string $value]'"
   } else {
      lappend values "$value"
   }
}
if {[llength $fields] == 0} {
   return [todomgr_pageout $conn user_add.html]
}
set problems [list]
if ![string compare $tags(userid) ""] {
   lappend problems "You must supply a desired userid to request a userid..."
}
if ![string compare $tags(password) ""] {
   lappend problems "You must supply a password."
}

if {[llength $problems] == 0} { # got this far...
   set row [ns_db select $db "select count(*) as ct from users where userid='[sql_safe_string $tags(userid)]'"]
   ns_db getrow $db $row
   if {[ns_set get $row ct] > 0} {
      lappend problems "Sorry, the userid <code>$tags(userid)</code> is taken.  Please choose something else."
   }
}

if {[llength $problems] > 0} {
   if {[llength $problems] == 1} {
      set tags(message) [lindex $problems 0]
   } else {
      set tags(message) "<ul>"
      foreach problem $problems {
         append tags(message) "<li> $problem\n"
      }
      append tags(message) "</ul>"
   }
   return [todomgr_pageout $conn user_add.html]
}

lappend fields permlevel
lappend values 0

set query "insert into users ("
append query [join $fields ", "]
append query ") values ("
append query [join $values ", "]
append query ")"
ns_db dml $db $query
return [ns_returnredirect $conn user?action=list]
         return
      }
      modify {
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set permlevel [ns_set get $userrow permlevel]

set fields [list]
set userid [ns_set get $form user]
if {$userid == ""} { set userid $user }
if {$permlevel < 3 && [string compare $userid $user]} {
   set tags(title) "Insufficient privilege"
   set tags(body)" Sorry, you don't have sufficient privilege to modify this user record."
   return [todomgr_pageout $conn message.html]
}

set tags(userid) $userid
if ![string compare $userid ""] {
   return [ns_returnredirect $conn user?action=list]
}

foreach field {password name email permlevel website} {
   set value [ns_set get $form $field]
   if {$value == ""} { continue }
   if {$field != "permlevel"} {
      lappend fields "$field='[sql_safe_string $value]'"
   } else {
      if {$permlevel < 3} { continue } 
      lappend fields "$field=$value"
   }
}

if {[llength $fields] > 0} {
   set query "update users set "
   append query [join $fields ", "]
   append query " where userid='[sql_safe_string $userid]'"
   ns_db dml $db $query
   if [string compare $userid $user] {
      return [ns_returnredirect $conn user?action=list]
   } else {
      return [ns_returnredirect $conn user?action=modify]
   }
}

set row [ns_db select $db "select * from users where userid='[sql_safe_string $userid]'"]
if {[ns_db getrow $db $row]} {
   foreach field {password name email userid permlevel website} {
      set tags($field) [ns_set get $row $field]
   }
   return [todomgr_pageout $conn user_mod.html]
}
set tags(title) "Unknown user $userid"
todomgr_pageout $conn user_list.html
         return
      }
      delete {
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
if {[ns_set get $userrow permlevel] < 3} {
   set tags(title) "Insufficient privileges"
   set tags(body) "Your permission level is lower than 3; a level of 3 is required to delete users.  Sorry."
   return [todomgr_pageout $conn message.html]
}

set size [ns_set size $form]
set deletes [list]
for {set i 0} {$i < $size} {incr i} {
   if {[string tolower [ns_set key $form $i]] == "delete"} {
      lappend deletes "'[sql_safe_string [ns_set value $form $i]]'"
   }
}
if {[llength $deletes] > 0} {
   set query "delete from users where userid="
   append query [join $deletes " or userid="]
   ns_db dml $db $query
}
return [ns_returnredirect $conn user?action=list]
         return
      }
   }

   set tags(title) "Unknown action"
   set tags(body) "What is this <code>$action</code> of which you speak?"
   todomgr_pageout $conn message.html
}
ns_register_proc GET $todomgr_root/login todomgr_login

proc todomgr_login {conn ignore} {
   set userid [ns_conn authuser $conn]
   if ![string compare $userid ""] {
      return [todomgr_pageout $conn nologin.html]
   }
   global todomgr_pool
   set db [ns_db gethandle $todomgr_pool]
   set row [ns_db select $db "select * from users where userid='[sql_safe_string $userid]'"]
   ns_db getrow $db $row
   foreach field {userid password name email website} {
      set tags($field) [ns_set get $row $field]
   }
   set form [ns_conn form $conn]
   if {$form == ""} {
set tags(title) "Current login user: $tags(userid) ($tags(name))"
return [todomgr_pageout $conn login.html]
   } else {
if [string compare [string tolower $userid] [string tolower [ns_set get $form user]]] {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}
set tags(title) "Current login user: $tags(userid) ($tags(name))"
return [todomgr_pageout $conn login.html]
   }
}
ns_register_proc GET $todomgr_root/keywords todomgr_keywords
ns_register_proc POST $todomgr_root/keywords todomgr_keywords

proc todomgr_keywords {conn ignore} {
   set form [ns_conn form $conn]
   if {$form == ""} { set form [ns_set create] }

   global todomgr_pool
   set db [ns_db gethandle $todomgr_pool]
set user [string tolower [ns_conn authuser $conn]]
if [string compare $user ""] {
   if [catch {set userrow [ns_db select $db "select * from users where userid='[sql_safe_string $user]'"]} result] {
      set tags(title) "Problem validating user"
      set tags(body) "The database returned an error while attempting to perform signon."
      append tags(body) "<br>The error returned was: <code>$result</code>"
      return [todomgr_pageout $conn message.html 500]
   }
   if {![ns_db getrow $db $userrow] || \
        [string compare [string tolower [ns_conn authpassword $conn]] \
                        [string tolower [ns_set get $userrow password]]]} {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"task list manager\""
      return [todomgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [todomgr_pageout $conn auth.html 401]
}

   set process [ns_set get $form process]
   if [string compare "" $process] {
if {[catch {set prow [ns_db select $db "select * from process where id='[sql_safe_string $process]'"]} result] || \
   ![ns_db getrow $db $prow]} {
   set tags(title) "Unknown process"
   set tags(body) "The process you specified (<code>$process</code>) cannot be found in the database."
   return [todomgr_pageout $conn message.html]
}

if [string compare $user [ns_set get $prow owner]] {
   if {[catch {set perm [ns_db select $db "select * from permission where process='[sql_safe_string $process]' and userid='[sql_safe_string $user]'"]} result] \
       || ![ns_db getrow $db $perm] || ![string match *p* [ns_set get $perm flags]]} {
      set tags(title) "Insufficient privilege"
      set tags(body) "You have not been granted the right to assign keywords to the project you specified."
      return [todomgr_pageout $conn message.html]
   }
}
set keywords [list]
if [string compare "" [ns_set get $form newkeyword]] {
   set row [ns_db select $db "select count(*) as ct from keyword where keyword='[sql_safe_string [ns_set get $form newkeyword]]'"]
   ns_db getrow $db $row
   if {[ns_set get $row ct] == 0} {
      lappend keywords [ns_set get $form newkeyword]
      ns_db dml $db "insert into permission (keyword, userid, flags) values ('[sql_safe_string [ns_set get $form newkeyword]]', '[sql_safe_string $user]', 'pmrt')"
   }
}
set size [ns_set size $form]
for {set i 0} {$i < $size} {incr i} {
   if ![string compare "keyword" [string tolower [ns_set key $form $i]]] {
      lappend keywords [ns_set value $form $i]
   }
}
if ![string compare $user [ns_set get $prow owner]] {
   set query "delete from keyword where process='[sql_safe_string $process]'"
   catch {ns_db dml $db $query} result
} else {
   set deletes [list]
   set query "select keyword.keyword as k from keyword, permission where keyword.keyword=permission.keyword and keyword.process='[sql_safe_string $process]' and permission.userid='[sql_safe_string $user]'"
   if ![catch {set row [ns_db select $db $query]} result] {
      while {[ns_db getrow $db $row]} {
         lappend deletes "'[sql_safe_string [ns_set get $row k]]'"
      }
   }
   if {[llength $deletes] > 0} {
      set query "delete from keyword where process='[sql_safe_string $process]' and (keyword="
      append query [join $deletes " or keyword="]
      append query ")"
      ns_db dml $db $query
   }
}
foreach word $keywords {
   ns_db dml $db "insert into keyword (keyword, process) values ('[sql_safe_string $word]', '[sql_safe_string $process]')"
}
ns_returnredirect $conn show?process=$process
      return
   }

   set userid [ns_set get $form user]
   set keyword [ns_set get $form keyword]
   if [string compare "" $userid] {
set query "select * from permission where keyword='[sql_safe_string $keyword]' and userid='[sql_safe_string $user]'"
if {[catch {set row [ns_db select $db $query]} result] \
    || ![ns_db getrow $db $row] \
    || ![string match *p* [ns_set get $row flags]]} {
   set tags(title) "Insufficient privilege"
   set tags(body) "Sorry, you don't have sufficient privilege to grant permissions to this keyword."
   return [todomgr_pageout $conn message.html]
}
set flags ""
foreach field {p r m t} {
   if [string compare "" [ns_set get $form $field]] { append flags $field }
}
set query "delete from permission where keyword='[sql_safe_string $keyword]' and userid='[sql_safe_string $userid]'"
ns_db dml $db $query

set query "insert into permission (keyword, userid, flags) values ('[sql_safe_string $keyword]', '[sql_safe_string $userid]', '$flags')"
ns_db dml $db $query
ns_returnredirect $conn keywords?keyword=$keyword
      return
   }

   if [string compare "" $keyword] {
set query "select * from permission where userid='[sql_safe_string $user]' and keyword='[sql_safe_string $keyword]' and flags like '%p%'"
if {[catch {set row [ns_db select $db $query]} result] || ![ns_db getrow $db $row]} {
   set tags(title) "Keyword not found"
   set tags(body) "The keyword you entered (<code>$keyword</code>) couldn't be found or you don't have administrative privileges to it."
   return [todomgr_pageout $conn message.html]
}

set tags(keyword) $keyword
set query "select * from keyword, process where keyword.process=process.id and keyword='[sql_safe_string $keyword]'"
set tags(processlist) ""
if [catch {set row [ns_db select $db $query]} result] {
   set tags(processlist) "<i>No processes attached</i>"
} else {
   while {[ns_db getrow $db $row]} {
      append tags(processlist) "<a href=show?process=[ns_set get $row id]>"
      append tags(processlist) "[ns_set get $row title]</a><br>\n"
   }
   if {$tags(processlist) == ""} {
      set tags(processlist) "<i>No processes attached</i>"
   }
}
set tags(userlist) ""
set userlist [list]
set query "select * from users, permission where permission.userid=users.userid and keyword='[sql_safe_string $keyword]' order by name"
if [catch {set row [ns_db select $db $query]} result] {
   set tags(userlist) "<i>No users attached</i>"
} else {
   while {[ns_db getrow $db $row]} {
      lappend userlist [ns_set get $row userid]
      foreach perm {p r m t} {
         set check$perm ""
         if [string match "*$perm*" [ns_set get $row flags]] {
            set check$perm " checked"
         }
      }
      append tags(userlist) "<form action=keywords method=post>"
      append tags(userlist) "<input type=hidden name=user value=\"[ns_set get $row userid]\">"
      append tags(userlist) "<input type=hidden name=keyword value=\"$keyword\">"
      append tags(userlist) "[ns_set get $row name] ([ns_set get $row userid])<br>"
      append tags(userlist) "<input name=p type=checkbox value=p$checkp>Admin"
      append tags(userlist) "<input name=r type=checkbox value=r$checkr>Priority"
      append tags(userlist) "<input name=m type=checkbox value=m$checkm>Modify"
      append tags(userlist) "<input name=t type=checkbox value=t$checkt>Task"
      append tags(userlist) "<input type=submit value=\"Update\">"
      append tags(userlist) "</form>"
   }
   if {$tags(userlist) == ""} {
      set tags(userlist) "<i>No users assigned</i>"
   }
}
set tags(newuserlist) ""
set query "select * from users where permlevel > 0 order by name"
if ![catch {set row [ns_db select $db $query]} result] {
   while {[ns_db getrow $db $row]} {
      if {[lsearch $userlist [ns_set get $row userid]] > -1} { continue }
      append tags(newuserlist) "<option value=\"[ns_set get $row userid]\">"
      append tags(newuserlist) "[ns_set get $row name] ([ns_set get $row userid])\n"
   }
}
todomgr_pageout $conn keyword.html
      return
   }

set query "select * from permission where userid='[sql_safe_string $user]' and flags like '%p%'"
set tags(title) "Keyword list"
set tags(body) ""
if ![catch {set row [ns_db select $db $query]} result] {
    while {[ns_db getrow $db $row]} {
      append tags(body) "<li><a href=keywords?keyword=[ns_set get $row keyword]>"
      append tags(body) "[ns_set get $row keyword]</a>\n"
   }
}
if [string compare $tags(body) ""] {
   set tags(body) "Click on a keyword to go to its management screen:<ul>$tags(body)</ul>"
} else {
   set tags(body) "You appear to have admin access to no keywords."
}
todomgr_pageout $conn message.html
}
proc wftk_interpret {db process workflow} {
   while {[llength $workflow] > 0} {
      set cmd [lindex $workflow 0]
      set workflow [lrange $workflow 1 end]

      switch [string range $cmd 0 0] {
         A {
set p [split [string range $cmd 2 end] -]

set fields [list id status created]
set values [list '[lindex $p 0]' 'active' '[now]']

lappend fields process
lappend values "'$process'"

lappend fields owner
set owner [lindex $p 1]
if ![string compare $owner "!user"] {
   upvar user user
   set owner $user
}
lappend values '[sql_safe_string $owner]'

lappend fields description
lappend values '[sql_safe_string [join [lrange $p 2 end] -]]'

set query "insert into task ("
append query [join $fields ", "]
append query ") values ("
append query [join $values ", "]
append query ")"

ns_db dml $db $query
         }
         L {
         }
         F {
set query "update process set status='complete' where id='$process'"
ns_db dml $db $query
         }
      }
   }
}
set todomgr_xmltools "/usr/local/AOLserver/vivtek/pages/xmltools"
set todomgr_datasheets "/usr/local/AOLserver/vivtek/pages/todomgr/datasheets"

proc datasheet_getvalue {process task number {active 0}} {
   global todomgr_datasheets
   global todomgr_xmltools
   set datasheet "$todomgr_datasheets/$process"
   if ![file exists $datasheet] return ""

   set loc datasheet
   if [string compare $task ""] { append loc ".task\[$task\]" }
   append loc ".data($number)"

   set data [exec $todomgr_xmltools/xmlsnip $loc $datasheet]
   if {0 == [regexp ^<(\[^<\]*)> $data tag bits]} { return "" }
   if  [string match */ $bits] {
      set data ""
      regsub /$ $bits "" bits
   } else {
      regsub ^<\[^<\]*> $data "" data
      regsub <\[^>\]*>$ $data "" data
   }
   set fields [list]
   set id ""
   set type ""
   set bits [split [join [lrange [split $bits] 1 end]] =]
   set name [lindex $bits 0]
   foreach bit [lrange $bits 1 end] {
      set bit [split $bit \"]
      lappend fields [list $name [lindex $bit 1]]
      switch $name {
         id { set id [lindex $bit 1] }
         type { set type [lindex $bit 1] }
      }
      set name [string trim [lindex $bit 2]]
   }
   switch $type {
      string { set html "<input name=\"$id\" value=\"$data\">" }
      text { set html "<textarea name=\"$id\" rows=5 cols=40>$data</textarea>" }
      default { set html "<input name=\"$id\" value=\"$data\">" }
   }
   return [list $id $type $data $html $fields]
}
proc datasheet_setvalue {process task name type value} {
   if ![string compare "" $process] { return }
   regsub -all "\"" $name ' name
   global todomgr_datasheets
   global todomgr_xmltools
   set datasheet "$todomgr_datasheets/$process"
   set loc datasheet
   if [string compare $task ""] { append loc ".task\[$task\]" }

   if ![file exists $datasheet] {
      exec $todomgr_xmltools/xmlcreate datasheet > $datasheet
   }
   if [string compare $task ""] {
      set taskloc [exec $todomgr_xmltools/xmlsnip -otl datasheet.task\[$task\] $datasheet]
      if ![string compare $taskloc ""] {
         exec mv $datasheet $datasheet.~
         set pipe [open "|$todomgr_xmltools/xmlinsert aftercontent datasheet $datasheet.~ > $datasheet" w]
         puts $pipe "<task id=\"$task\">"
         puts $pipe "</task>"
         close $pipe
      }
   }
   set dataloc [exec $todomgr_xmltools/xmlsnip -otl $loc.data\[$name\] $datasheet]
   if [string compare $dataloc ""] {
      exec mv $datasheet $datasheet.~
      regsub -all " " $name "\\ " n
      set cmd "|$todomgr_xmltools/xmlreplace -m $loc.data\[$n\] $datasheet.~ > $datasheet"
      set pipe [open $cmd w]
      puts -nonewline $pipe $value
      close $pipe
   } else {
      exec mv $datasheet $datasheet.~
      set cmd "|$todomgr_xmltools/xmlinsert aftercontent $loc $datasheet.~ > $datasheet"
      set pipe [open $cmd w]
      puts $pipe "<data id=\"$name\" type=\"$type\">$value</data>"
      close $pipe
   }
}
proc datasheet_showdata {action back process task} {
   set retval ""
   set i 0

   while (1) {
      set d [datasheet_getvalue $process $task $i]
      incr i
      if ![string compare "" [lindex $d 0]] { break }
      if {$action == "edit"} {
         append retval "<tr><td>[lindex $d 0]:</td><td>[lindex $d 3]</td></tr>\n"
      } else {
         append retval "<tr><td>[lindex $d 0]:</td><td>[lindex $d 2]</td></tr>\n"
      }
   }

   if {$action == "edit" && $retval != ""} {
      set retval "<form action=setvalue method=post>\n$retval"
      append retval "<input type=hidden name=process value=\"$process\">\n"
      append retval "<input type=hidden name=task value=\"$task\">\n"
      append retval "<input type=hidden name=back value=\"$back\">\n"
      append retval "<tr><td colspan=2><center>"
      append retval "<input type=submit value=\"Update values\"></center></td></tr>\n"
      append retval "</form>\n"
   }

   return $retval
}
ns_register_proc GET  $todomgr_root/setvalue setvalue
ns_register_proc POST $todomgr_root/setvalue setvalue
proc setvalue {conn ignore} {
   set form [ns_conn form $conn]
   if {$form == ""} { 
      set tags(title) "No parameters given"
      set tags(body) "You can't set a value without giving the value."
      return [todomgr_pageout $conn message.html]
   }
   set process [ns_set get $form process]
   set task [ns_set get $form task]
   set newname [ns_set get $form newname]
   if [string compare "" $newname] {
      datasheet_setvalue $process $task $newname [ns_set get $form type] ""
      return [ns_returnredirect $conn [ns_set get $form back]
   }

   set size [ns_set size $form]
   for {set i 0} {$i < $size} {incr i} {
      if {-1 < [lsearch {process task back newname type} [ns_set key $form $i]]} { continue }

      datasheet_setvalue $process $task [ns_set key $form $i] "" [ns_set value $form $i]
   }
   return [ns_returnredirect $conn [ns_set get $form back]]
}
