# Resana

Somewhat of a Windows Task Manager clone, Resana provides live statistics of system information.

This program is a experiment and work-in-progress that allows me to learn more about software engineering and development, system design, databases, and offers an experience in other advanced topics such as multi-threading and parallel processing.

---

### Currently supported Data

* **CPU**
  * Total CPU load
  * CPU load of each logical processor
  * CPU demand of current process  
* **Memory** _(Physica/Virtual_)
  * Total memory
  * Memory used
  * Available memory
  * Amount used by current process
* **Process Information**
  * Executable name
  * Process and parent process IDs
  * Thread count
  * Priority class

---

### Road map

* UI
  * Resource analyzer panel:
    * Rework update interval speed selection
    * Process panel:
      * Get process memory usage and module ID data working
      * Get processes sortable by name
      * Get processes findable (i.e. searchable)
    * Performance panel:
      * Show total CPU load usage in table header (or something)
      * Make logical processor table expandable upon clicking total usage header
* Future updates:
  * Implement an event system. This is mainly do to the increasing demand of event notifications such as the consistent and sychronous updating of panels and panel objects.

---

  ### Bugs
  * Fix 'View' menu in Process Details to stop it from taking context when hovered
  * Closing Resource Analyzer panel crashes program
  * Program exits after waiting for ending threads to join. This can take a few seconds and can be improved with an event system _(coming in the future)_
  * Processes are not deselected when clicking a selected process.
