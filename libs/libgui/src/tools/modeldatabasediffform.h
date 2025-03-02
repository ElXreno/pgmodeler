/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2021 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

/**
\ingroup libgui
\class ModelDatabaseDiffForm
\brief Implements the operations to compare and generate a diff between model and database via form.
*/

#ifndef MODEL_DATABASE_DIFF_FORM
#define MODEL_DATABASE_DIFF_FORM

#include "ui_modeldatabasediffform.h"
#include "modelsdiffhelper.h"
#include "databaseimporthelper.h"
#include "modelexporthelper.h"
#include "utils/syntaxhighlighter.h"
#include "utils/htmlitemdelegate.h"
#include "widgets/numberedtexteditor.h"
#include "settings/baseconfigwidget.h"
#include "widgets/fileselectorwidget.h"
#include "widgets/objectsfilterwidget.h"
#include <QThread>

class ModelDatabaseDiffForm: public BaseConfigWidget, public Ui::ModelDatabaseDiffForm {
	private:
		Q_OBJECT

		/*! \brief Indicates if the full output generated during the process should be displayed
		 * When this attribute is true, only errors and some key info messages are displayed. */
		static bool low_verbosity;

		static map<QString, attribs_map> config_params;

		QEventLoop event_loop;

		bool is_adding_new_preset;

		NumberedTextEditor *sqlcode_txt;

		FileSelectorWidget *file_sel;

		ObjectsFilterWidget *pd_filter_wgt;

		//! \brief Custom delegate used to paint html texts in output tree
		HtmlItemDelegate *htmlitem_del;

		//! \brief Syntax highlighter used on the diff preview tab
		SyntaxHighlighter *sqlcode_hl;

		//! \brief Helper that will execute the diff between models
		ModelsDiffHelper *diff_helper;

		//! \brief Helper that will execute the database import
		DatabaseImportHelper *import_helper, *src_import_helper;

		//! \brief Helper that will execute the diff export to database
		ModelExportHelper *export_helper;

		//! \brief Threads that will execute each step: import, diff, export
		QThread *import_thread, *diff_thread, *export_thread, *src_import_thread;

		//! \brief Tree items generated in each diff step
		QTreeWidgetItem *import_item, *diff_item, *export_item, *src_import_item;

		//! \brief Stores the objects filtered from the database model
		vector<BaseObject *> filtered_objs;

		/*! \brief This is the model used in the diff process representing the source.
		 * It can be the modelo loaded from file or a representation of the source database (when comparing two dbs) */
		DatabaseModel *source_model,

		//! \brief This is the model loaded from file
		*loaded_model,

		//! \brief This is the model generated by the reverse engineering step
		*imported_model;

		//! \brief Connection used to export the diff to database
		Connection *export_conn;

		//! \brief PostgreSQL version used by the diff process
		QString pgsql_ver;

		int diff_progress, curr_step, total_steps;

		bool process_paused;

		void closeEvent(QCloseEvent *event);
		void showEvent(QShowEvent *);

		//! \brief Creates the helpers and threads
		void createThread(unsigned thread_id);

		//! \brief Destroy the helpers and threads
		void destroyThread(unsigned thread_id);

		//! \brief Destroy the imported model
		void destroyModel();

		void clearOutput();
		void resetForm();
		void resetButtons();
		void saveDiffToFile();
		void finishDiff();

		//! \brief Returns true when one or more threads of the whole diff process are running.
		bool isThreadsRunning();

		//! \brief Constants used to reference the thread/helper to be handled in createThread() and destroyThread()
		static constexpr unsigned SrcImportThread=0,
		ImportThread=1,
		DiffThread=2,
		ExportThread=3;

		//! \brief Applies the loaded configurations to the form. In this widget only list the loaded presets
		virtual void applyConfiguration();

		//! \brief Loads a set of configurations from a file
		virtual void loadConfiguration();

		//! \brief Saves the current settings to a file
		virtual void saveConfiguration();

		void togglePresetConfiguration(bool toggle, bool is_edit = false);

		void enablePresetButtons();

		/*! \brief When performing a partial diff between a model and database this method fills a vector with the
		 * filtered objects in the source database model */
		void getFilteredObjects(vector<BaseObject *> &objects);

		/*! \brief When performing a partial diff between two databases this method fills a map with the
		 * filtered objects (type -> oids) in the database */
		void getFilteredObjects(map<ObjectType, vector<unsigned> > &obj_oids);

	public:
		ModelDatabaseDiffForm(QWidget * parent = nullptr, Qt::WindowFlags flags = Qt::Widget);
		virtual ~ModelDatabaseDiffForm();

		//! \brief Makes the form behaves like a QDialog by running it from an event loop. The event loop is finished when the user clicks close
		void exec();

		void setModelWidget(ModelWidget *model_wgt);

		//! \brief Defines if all the output generated during the import process should be displayed
		static void setLowVerbosity(bool value);

	private slots:
		void listDatabases();
		void enableDiffMode();
		void generateDiff();
		void cancelOperation(bool cancel_by_user);
		void updateProgress(int progress, QString msg, ObjectType obj_type, QString cmd="");
		void updateDiffInfo(ObjectsDiffInfo diff_info);
		void captureThreadError(Exception e);
		void handleImportFinished(Exception e);
		void handleDiffFinished();
		void handleExportFinished();
		void handleErrorIgnored(QString err_code, QString err_msg, QString cmd);
		void importDatabase(unsigned thread_id);
		void diffModels();
		void exportDiff(bool confirm=true);
		void filterDiffInfos();
		void loadDiffInSQLTool();
		void selectPreset();
		void removePreset();
		void savePreset();
		void enablePartialDiff();
		void enableFilterByDate();
		void applyPartialDiffFilters();
		void generateFiltersFromChangelog();

		//! \brief Destroy the current configuration file and makes a copy of the default one located at conf/defaults
		virtual void restoreDefaults();

	signals:
		/*! \brief This signal is emitted whenever the user changes the connections settings
		within this widget without use the main configurations dialog */
		void s_connectionsUpdateRequest();

		/*! \brief This signal is emitted whenever the user wants to load the generated diff in the sql tool
		 * The signal contains the connection id, the database name and the temp filename that is generated containing
		 * the commands to be loaded */
		void s_loadDiffInSQLTool(QString conn_id, QString database, QString sql_file);
};

#endif
