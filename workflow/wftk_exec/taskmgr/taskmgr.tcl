# Task manager, take 2.
# Copyright (c) 2000, Vivtek.
# Released under the terms of the GNU license.

set wftk_home /usr/local/AOLserver/vivtek/pages/workflow/wftk_exec/
set taskmgr_home ${wftk_home}/taskmgr/
set taskmgr_root /workflow/wftk_exec/taskmgr
set taskmgr_pool wftk
set taskmgr_auth_realm "wftk workflow"
set pdm_ext ".cgi"
set taskmgr_datasheets "/usr/local/AOLserver/vivtek/pages/workflow/working/datasheets"
ns_register_proc GET  $taskmgr_root/create taskmgr_create
ns_register_proc POST $taskmgr_root/create taskmgr_create
ns_register_proc GET  $taskmgr_root/start taskmgr_start
ns_register_proc POST $taskmgr_root/start taskmgr_start
ns_register_proc GET  $taskmgr_root/show taskmgr_show
ns_register_proc POST $taskmgr_root/show taskmgr_show
ns_register_proc GET  $taskmgr_root/complete taskmgr_complete
ns_register_proc POST $taskmgr_root/complete taskmgr_complete
ns_register_proc GET  $taskmgr_root/reject taskmgr_reject
ns_register_proc POST $taskmgr_root/reject taskmgr_reject
ns_register_proc GET  $taskmgr_root/update taskmgr_update
ns_register_proc POST $taskmgr_root/update taskmgr_update

ns_register_proc GET  $taskmgr_root/overview taskmgr_overview
proc taskmgr_create {conn ignore} {
global taskmgr_pool
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
}
set form [ns_conn form $conn]
if {$form == ""} {
set row [ns_db select $db "select * from process"]
set tags(processlist) "<option value=\"\">Select a process if applicable\n"
while {[ns_db getrow $db $row]} {
   append tags(processlist) "<option value=\"[ns_set get $row id]\">[ns_set get $row title]\n"
}
taskmgr_pageout $conn fresh_task.html
  return
}
if {[ns_set get $form what] == "task"} {
  if ![string compare [ns_set get $form description] ""] {
set process [ns_set get $form process]
if ![string compare "" $process] {
   return [taskmgr_pageout $conn newtasknoproc.html]
} 
set tags(process) $process
set prow [ns_db select $db "select * from process where id='[sql_safe_string $process]'"]
if ![ns_db getrow $db $prow] {
   set tags(title) "Can't create task for nonexistent process"
   set tags(body) "The process code you entered (<code>$process</code>) can't be found in the database."
   return [taskmgr_pageout $conn message.html]
}
set tags(title) [ns_set get $prow title]
if {[string compare $user [ns_set get $prow owner]] && ![perms_check $user [ns_set get $prow groupown] taskadd]} {
   set tags(body) "You don't have sufficient privileges to request tasks for <i>$tags(title)</i>"
   set tags(title) "Insufficent privilege"
   return [taskmgr_pageout $conn message.html]
}
set tags(userlist) "<option value=\"$user\">Select a requestee here\n"
append tags(userlist) [perms_userselect [ns_set get $prow groupown] own]
 
taskmgr_pageout $conn newtask.html
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
return [taskmgr_pageout $conn newprocess.html]
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

  taskmgr_pageout $conn message.html
}
}

proc taskmgr_start {conn ignore} {
global taskmgr_pool
global taskmgr_home
global wftk_home
global pdm_ext
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
}
set form [ns_conn form $conn]
if {$form == ""} { set form [ns_set create] }

if [string compare "" [ns_set get $form procdef]] {
set tags(procdef) [ns_set get $form procdef]
set pipe [open "|${wftk_home}pdm/pdm$pdm_ext starter $tags(procdef)" "r"]
set tags(title) [gets $pipe]
set tags(version) [gets $pipe]
set tags(list) [read $pipe]
close $pipe

return [taskmgr_pageout $conn start.html]
}
if [string compare "" [ns_set get $form start]] {
set process $conn[now tag]
global taskmgr_datasheets
set pipe [open "|${wftk_home}pdm/pdm$pdm_ext datasheet [ns_set get $form start] [ns_set get $form ver] $process" "r"]
set datasheet [read $pipe]
close $pipe

set pipe [open "$taskmgr_datasheets/$process" "w"]
puts $pipe $datasheet
close $pipe

set size [ns_set size $form]
for {set i 0} {$i < $size} {incr i} {
   if {-1 < [lsearch {start ver} [ns_set key $form $i]]} { continue }
   datasheet_setvalue $process "" [ns_set key $form $i] "" [ns_set value $form $i]
}

set pipe [open "|${taskmgr_home}wftk start $process" "r"]
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

global wftk_home
set pipe [open "|${wftk_home}pdm/pdmlist $user start" "r"]
set procdefs [split [read $pipe] "\n"]
close $pipe

set tags(list) ""
set state 0

foreach line $procdefs {
   switch $state {
      0 {
         if [string compare $line ""] {
            set line [split $line :]
            append tags(list) "<li><a href=\"start?procdef=[lindex $line 0]\">[lindex $line 1]</a><br>\n"
            set state 1
         }
      }
      1 {
        if [string compare $line "EOF"] {
            append tags(list) "$line\n"
        } else {
            set state 0
        }
      }
   }
}

return [taskmgr_pageout $conn startlist.html]
}

proc taskmgr_show {conn ignore} {
set form [ns_conn form $conn]
if {$form == ""} {
   set tags(title) "Nothing to show"
   set tags(body) "You need to select something to show."
   return [taskmgr_pageout $conn message.html]
}
global taskmgr_pool
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
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
   return [taskmgr_pageout $conn message.html]
}

if [ns_db getrow $db $row] {
   foreach field {process status priority owner description created sched_date sched_time} {
      set tags($field) [ns_set get $row $field]
   }
   set tags(processid) $tags(process)
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
} elseif ![string compare [ns_set get $form mode] edit] {
   set perms_or [perms_or $user groupown taskadd]

   set query "select * from process where status <> 'complete' and "
   append query "(owner='[sql_safe_string $user]' $perms_or) order by title"
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
} else {
   set tags(process) "<i>none</i>\n"
}
set owner $tags(owner) 
if {$owner != ""} {
   if [catch {
      set pipe [open "|${wftk_home}user/user listuser $owner" "r"]
      set ownerinfo [split [read $pipe] "\n"]
      close $pipe} result ] {
      set tags(ownertag) "<i>Unknown owner</i>"
   } else {
      set ownerrow [ns_set create]
      foreach info $ownerinfo {
         set info [split $info :]
         ns_set put $ownerrow [lindex $info 0] [join [lrange $info 1 end] :]
      }

      if [string compare "" [ns_set get $ownerrow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $ownerrow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $ownerrow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $ownerrow fullname]] {
         append tags(ownertag) " ([ns_set get $ownerrow fullname])"
      }
   }
}
      set tags(updatelink) ""
      set tags(isowner) 0
      if ![string compare $tags(owner) $user] {
         set tags(isowner) 1
         set tags(updatelink) "show?task=$task&back=$back&mode=edit"
         set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
         if [string compare "" $tags(processid)] {
            set tags(taskdata) [datasheet_showdata edit $tags(backhere) $tags(process) $task]
         } else {
            set tags(taskdata) ""
         }
      }
      return [taskmgr_pageout $conn taskhome.html]
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
} elseif ![string compare [ns_set get $form mode] edit] {
   set perms_or [perms_or $user groupown taskadd]

   set query "select * from process where status <> 'complete' and "
   append query "(owner='[sql_safe_string $user]' $perms_or) order by title"
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
} else {
   set tags(process) "<i>none</i>\n"
}
set owner $tags(owner) 
if {$owner != ""} {
   if [catch {
      set pipe [open "|${wftk_home}user/user listuser $owner" "r"]
      set ownerinfo [split [read $pipe] "\n"]
      close $pipe} result ] {
      set tags(ownertag) "<i>Unknown owner</i>"
   } else {
      set ownerrow [ns_set create]
      foreach info $ownerinfo {
         set info [split $info :]
         ns_set put $ownerrow [lindex $info 0] [join [lrange $info 1 end] :]
      }

      if [string compare "" [ns_set get $ownerrow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $ownerrow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $ownerrow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $ownerrow fullname]] {
         append tags(ownertag) " ([ns_set get $ownerrow fullname])"
      }
   }
}
      set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
      if [string compare "" $tags(processid)] {
         set tags(taskdata) [datasheet_showdata view $tags(backhere) $tags(processid) $task]
      } else {
         set tags(taskdata) ""
      }
      return [taskmgr_pageout $conn task.html]
   }
} else {
   set tags(title) "Task $task unknown"
   set tags(body) "The task ID <code>$task</code> is not in the database.  Sorry."
   return [taskmgr_pageout $conn message.html]
}
   return
}
set process [ns_set get $form process]
if {$process != ""} {
if {![string compare $process all] || ![string compare $process mine]} {
set filter active
if ![string compare [ns_set get $form filter] complete] {
   set filter complete
}
switch $filter {
   active { set statuswhere "status <> 'complete'"; set filterword "active" }
   complete { set statuswhere "status='complete'"; set filterword "completed" }
}
set perms_or [perms_or [ns_set get $userrow name] groupown]
set tags(header) "<tr bgcolor=cccccc><td>Process</td>"
if ![string compare $process all] {
   set tags(title) "All $filterword processes"
   append tags(header) "<td>Owner</td>"
   set    query "select * from process "
   append query   "where $statuswhere and (owner='[sql_safe_string $user]' "
   append query   "or groupown='' or groupown is null $perms_or) "
   append query "order by title"
} else {
   set tags(title) "My $filterword processes"
   set query "select * from process where $statuswhere and owner='[sql_safe_string $user]' order by title"
}
append tags(header) "<td>Status</td></tr>"

set tags(table) ""
set rows 0
if ![catch {set row [ns_db select $db $query]} result] {
   while {[ns_db getrow $db $row]} {
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
      if ![string compare $user [ns_set get $row owner]] {
         append tags(table) "(<a href=\"show?process=[ns_set get $row id]\">modify</a>)</td>"
      }

      if [string compare $process mine] {
         append tags(table) "<td>[ns_set get $row owner]</td>"
      }

      append tags(table) "<td>[ns_set get $row status]</td></tr>\n"
   }
}
if {$rows == 0} {
   set tags(body) "<tr><td colspan=3><i>No processes found</i></td></tr>"
}

return [taskmgr_pageout $conn processlist.html]
}
if [catch {set row [ns_db select $db "select * from process where id='$process'"]} result] {
   set tags(title) "Process $process unknown"
   set tags(body) "The process ID <code>$process</code> is not in the database.  Sorry."
   return [taskmgr_pageout $conn message.html]
}

if [ns_db getrow $db $row] {
   foreach field {title owner description started} {
      set tags($field) [ns_set get $row $field]
   }
   set tags(process) $process
set owner $tags(owner) 
if {$owner != ""} {
   if [catch {
      set pipe [open "|${wftk_home}user/user listuser $owner" "r"]
      set ownerinfo [split [read $pipe] "\n"]
      close $pipe} result ] {
      set tags(ownertag) "<i>Unknown owner</i>"
   } else {
      set ownerrow [ns_set create]
      foreach info $ownerinfo {
         set info [split $info :]
         ns_set put $ownerrow [lindex $info 0] [join [lrange $info 1 end] :]
      }

      if [string compare "" [ns_set get $ownerrow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $ownerrow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $ownerrow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $ownerrow fullname]] {
         append tags(ownertag) " ([ns_set get $ownerrow fullname])"
      }
   }
}
   set tags(backhere) "[ns_conn url $conn]?[ns_conn query $conn]"
   set tags(processdata) [datasheet_showdata edit $tags(backhere) $process ""]
   return [taskmgr_pageout $conn process.html]
} else {
   set tags(title) "Process $process unknown"
   set tags(body) "The process ID <code>$process</code> is not in the database.  Sorry."
   return [taskmgr_pageout $conn message.html]
}
   return
}

set tags(title) "Nothing to show"
set tags(body) "You need to select something to show."
taskmgr_pageout $conn message.html
}

proc taskmgr_complete {conn ignore} {
global taskmgr_pool
global taskmgr_home
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
}
set form [ns_conn form $conn]
set task ""
if {$form != ""} { set task [ns_set get $form task] }
if {$task == ""} {
   set tags(title) "No task specified"
   set tags(body) "Task completion requires a task identifier."
   return [taskmgr_pageout $conn message.html]
}
set query "select * from task where id='[sql_safe_string $task]'"
if {[catch {set row [ns_db select $db $query]} result] || ![ns_db getrow $db $row]} {
   set tags(title) "Task unknown"
   set tags(body) "The task you specified (<code>$task</code>) could not be found in the database."
   return [taskmgr_pageout $conn message.html]
}

if [string compare $user [ns_set get $row owner]] {
   set tags(title) "Insufficient privilege"
   set tags(body) "You are not the owner of this task.  Only the owner may complete a task."
   return [taskmgr_pageout $conn message.html]
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
   return [taskmgr_pageout $conn message.html]
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
   return [taskmgr_pageout $conn message.html]
}
if [string compare "" [ns_set get $row definition]] {
   set pipe [open "|${taskmgr_home}wftk complete $process $task" "r"]
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

proc taskmgr_reject {conn ignore} {
global taskmgr_pool
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
}
set form [ns_conn form $conn]
set task ""
if {$form != ""} { set task [ns_set get $form task] }
if {$task == ""} {
   set tags(title) "No task specified"
   set tags(body) "Task rejection requires a task identifier."
   return [taskmgr_pageout $conn message.html]
}
set query "select * from task where id='[sql_safe_string $task]'"
if {[catch {set row [ns_db select $db $query]} result] || ![ns_db getrow $db $row]} {
   set tags(title) "Task unknown"
   set tags(body) "The task you specified (<code>$task</code>) could not be found in the database."
   return [taskmgr_pageout $conn message.html]
}

if [string compare $user [ns_set get $row owner]] {
   set tags(title) "Insufficient privilege"
   set tags(body) "You are not the owner of this task.  Only the owner may reject a task."
   return [taskmgr_pageout $conn message.html]
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
   return [taskmgr_pageout $conn message.html]
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
   return [taskmgr_pageout $conn message.html]
}
if [string compare "" [ns_set get $row definition]] {
   set tags(title) "wftk must handle rejection"
   set tags(body) "This task must be rejected by the wftk engine."
   return [taskmgr_pageout $conn message.html]
}
set query "update task set status='rejected' where id='[sql_safe_string $task]'"
ns_db dml $db $query

if [string compare [ns_set get $form back] ""] {
   return [ns_returnredirect $conn [ns_set get $form back]]
}
set tags(title) "Task complete"
set tags(body) "The task has been marked as complete"
return [taskmgr_pageout $conn message.html]
}

proc taskmgr_update {conn ignore} {
set form [ns_conn form $conn]
if {$form == ""} {
   set tags(title) "Nothing to update"
   set tags(body) "You need to specify something to update."
   return [taskmgr_pageout $conn message.html]
}
global taskmgr_pool
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
}

set task [ns_set get $form task]
if {$task != ""} {
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
   taskmgr_pageout $conn message.html
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
   taskmgr_pageout $conn message.html
}
return
}

set tags(title) "Nothing to update"
set tags(body) "You need to specify either a task or a process to update."
taskmgr_pageout $conn message.html
}

proc taskmgr_overview {conn ignore} {
global taskmgr_pool
set db [ns_db gethandle $taskmgr_pool]
set user [string tolower [ns_conn authuser $conn]]
global taskmgr_auth_realm
global wftk_home
if [string compare $user ""] {
   set pipe [open "|${wftk_home}user/user auth [string tolower [ns_conn authuser $conn]] [string tolower [ns_conn authpassword $conn]]" "r"]
   set userinfo [split [read $pipe] \n]
   close $pipe

   if ![llength $userinfo] {
      ns_set put [ns_conn outputheaders $conn WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
      return [taskmgr_pageout $conn auth.html 401]
   }
} else {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"$taskmgr_auth_realm\""
   return [taskmgr_pageout $conn auth.html 401]
}

set userrow [ns_set create]
foreach datum $userinfo {
   set datum [split $datum :]
   ns_set put $userrow [lindex $datum 0] [join [lrange $datum 1 end] :]
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
      return [taskmgr_pageout $conn message.html]
   }
   foreach field {title description owner} { set tags($field) [ns_set get $prow $field] }

set owner $tags(owner) 
if {$owner != ""} {
   if [catch {
      set pipe [open "|${wftk_home}user/user listuser $owner" "r"]
      set ownerinfo [split [read $pipe] "\n"]
      close $pipe} result ] {
      set tags(ownertag) "<i>Unknown owner</i>"
   } else {
      set ownerrow [ns_set create]
      foreach info $ownerinfo {
         set info [split $info :]
         ns_set put $ownerrow [lindex $info 0] [join [lrange $info 1 end] :]
      }

      if [string compare "" [ns_set get $ownerrow email]] {
         set tags(ownertag) "<a href=\"mailto:[ns_set get $ownerrow email]\">"
      } else {
         set tags(ownertag) ""
      }
      append tags(ownertag) $owner
      if [string compare "" [ns_set get $ownerrow email]] {
         append tags(ownertag) "</a>"
      }
      if [string compare "" [ns_set get $ownerrow fullname]] {
         append tags(ownertag) " ([ns_set get $ownerrow fullname])"
      }
   }
}

   if ![string compare $tags(owner) $user] {
      set tags(modlink) "show?process=$process"
      append tags(modlink) "&back=[ns_urlencode [ns_conn url $conn]?[ns_conn query $conn]]"
      if [perms_check $user [ns_set get $prow groupown] taskadd] {
         set tags(requestlink) "create?what=task&process=$process"
         append tags(requestlink) "&back=[ns_urlencode [ns_conn url $conn]?[ns_conn query $conn]]"
      } else {
         set tags(requestlink) ""
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
   taskmgr_pageout $conn processhome.html
} else {
   taskmgr_pageout $conn overview.html
}
}
proc taskmgr_pageout {conn file {status 200}} {
   upvar tags tags
   global taskmgr_home
   set fn $taskmgr_home/$file
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
         regsub -all -nocase \\\[##$tag##\\\] $line [escape_ampersand [taskmgr_pageout_tag_value $tag]] line
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
proc taskmgr_pageout_tag_value {tag} {
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
ns_register_proc GET $taskmgr_root/login taskmgr_login

proc taskmgr_login {conn ignore} {
   set userid [ns_conn authuser $conn]
   if ![string compare $userid ""] {
      return [taskmgr_pageout $conn nologin.html]
   }
   global taskmgr_pool
   set db [ns_db gethandle $taskmgr_pool]
   set row [ns_db select $db "select * from users where userid='[sql_safe_string $userid]'"]
   ns_db getrow $db $row
   foreach field {userid password name email website} {
      set tags($field) [ns_set get $row $field]
   }
   set form [ns_conn form $conn]
   if {$form == ""} {
set tags(title) "Current login user: $tags(userid) ($tags(name))"
return [taskmgr_pageout $conn login.html]
   } else {
if [string compare [string tolower $userid] [string tolower [ns_set get $form user]]] {
   ns_set put [ns_conn outputheaders $conn] WWW-Authenticate "BASIC realm=\"task list manager\""
   return [taskmgr_pageout $conn auth.html 401]
}
set tags(title) "Current login user: $tags(userid) ($tags(name))"
return [taskmgr_pageout $conn login.html]
   }
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
proc perms_or {user sql_field {perm "view"}} {
   global wftk_home
   set ret ""

   set pipe [open "|${wftk_home}user/user groups $user $perm" "r"]
   set groups [split [read $pipe] "\n"]
   close $pipe

   foreach group $groups {
      append ret " or $sql_field = '[sql_safe_string [lindex [split $group :] 0]]'"
   }

   return $ret
}
proc perms_check {user group {perm "view"}} {
   if ![string compare $group ""] { return 1 }
   global wftk_home
   set pipe [open "|${wftk_home}user/user permgroup $user $group $perm" "r"]
   set perm [read $pipe]
   close $pipe

   return [string match OK* $perm]
}
proc perms_userselect {group {perm "view"}} {
   if ![string compare $group ""] { set group "everybody" }

   global wftk_home
   set pipe [open "|${wftk_home}user/user userlist $group $perm" "r"]
   set info [split [read $pipe] "\n"]
   close $pipe

   set ret ""
   foreach i $info {
      set i [split $i :]

      set name [join [lrange $i 2 end] :]
      if ![string compare $name ""] {
         set name [lindex $i 0]
      } else {
         append name " ([lindex $i 0])"
      }
      append ret "<option value=\"[lindex $i 0]\">$name\n"
   }

   return $ret
}

proc datasheet_getvalue {process task number {active 0}} {
   global taskmgr_datasheets
   set datasheet "$taskmgr_datasheets/$process"
   if ![file exists $datasheet] return ""

   set loc datasheet
   if [string compare $task ""] { append loc ".task\[$task\]" }
   append loc ".data($number)"

   set data [exec ${wftk_home}xmltools/xmlsnip $loc $datasheet]
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
   global taskmgr_datasheets
   set datasheet "$taskmgr_datasheets/$process"
   set loc datasheet
   if [string compare $task ""] { append loc ".task\[$task\]" }

   if ![file exists $datasheet] {
      exec ${wftk_home}xmltools/xmlcreate datasheet > $datasheet
   }
   if [string compare $task ""] {
      set taskloc [exec ${wftk_home}xmltools/xmlsnip -otl datasheet.task\[$task\] $datasheet]
      if ![string compare $taskloc ""] {
         exec mv $datasheet $datasheet.~
         set pipe [open "|${wftk_home}xmltools/xmlinsert aftercontent datasheet $datasheet.~ > $datasheet" w]
         puts $pipe "<task id=\"$task\">"
         puts $pipe "</task>"
         close $pipe
      }
   }
   set dataloc [exec ${wftk_home}xmltools/xmlsnip -otl $loc.data\[$name\] $datasheet]
   if [string compare $dataloc ""] {
      exec mv $datasheet $datasheet.~
      regsub -all " " $name "\\ " n
      set cmd "|${wftk_home}xmltools/xmlreplace -m $loc.data\[$n\] $datasheet.~ > $datasheet"
      set pipe [open $cmd w]
      puts -nonewline $pipe $value
      close $pipe
   } else {
      exec mv $datasheet $datasheet.~
      set cmd "|${wftk_home}xmltools/xmlinsert aftercontent $loc $datasheet.~ > $datasheet"
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
ns_register_proc GET  $taskmgr_root/setvalue setvalue
ns_register_proc POST $taskmgr_root/setvalue setvalue
proc setvalue {conn ignore} {
   set form [ns_conn form $conn]
   if {$form == ""} { 
      set tags(title) "No parameters given"
      set tags(body) "You can't set a value without giving the value."
      return [taskmgr_pageout $conn message.html]
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
