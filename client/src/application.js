import AppBar from 'material-ui/AppBar';
import CasesPanel from './casesPanel';
import DatasetPanel from './datasetPanel';
import DisplayPanel from './displayPanel';
import Drawer from 'material-ui/Drawer';
import { FormControl, FormHelperText } from 'material-ui/Form';
import Input, { InputLabel } from 'material-ui/Input';
import { MenuItem } from 'material-ui/Menu';
import React from 'react';
import Paper from 'material-ui/Paper';
import PropTypes from 'prop-types';
import Select from 'material-ui/Select';
import Toolbar from 'material-ui/Toolbar';
import Typography from 'material-ui/Typography';
import WebGLWindow from './webGLWindow';
import Workspace from './workspace';
import { withStyles } from 'material-ui/styles';


const drawerWidth = 260;
const styles = theme => ({
  root: {
    flexGrow: 1,
    zIndex: 1,
    overflow: 'hidden',
    position: 'relative',
    display: 'flex',
    height: 'calc(100vh)',
  },
  appBar: {
    zIndex: theme.zIndex.drawer + 1,
  },
  drawerPaper: {
    position: 'relative',
    width: drawerWidth,
    overflowX: 'hidden',
  },
  content: {
    flexGrow: 1,
    minWidth: 0,
    backgroundColor: '#eef',   
  },
  workspace: {
    display: 'grid',
    height: 'calc(100vh - 64px)',
    gridTemplateColumns: '1fr 1fr',
    gridTemplateRows: '1fr',
    gridGap: '0em',
  },
  toolbar: theme.mixins.toolbar,
})


/**
 * The top level dSpaceX client component.
 */
class Application extends React.Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    console.log('Initializing Application...');
  }

  componentDidMount() {
    console.log('Application Ready.');
  }

  render() {
    const { classes } = this.props;
    return (
      <div className={classes.root}>
        <AppBar position='absolute' className={classes.appBar}>
          <Toolbar>
            <Typography variant="title" color="inherit">
              dSpaceX
            </Typography>
          </Toolbar>
        </AppBar>
        <Drawer variant='permanent' 
                classes={{ paper: classes.drawerPaper }}>
          { /* Add div to account for menu bar */ }
          <div className={classes.toolbar} />
          <DatasetPanel/>
          <CasesPanel/>
          <DisplayPanel/>
        </Drawer>
        <Workspace className={classes.content}>
            { /* Add div to account for menu bar */ }
            <div className={classes.toolbar} />
            <div className={classes.workspace}>
              <WebGLWindow/>
              <WebGLWindow/>
            </div>
        </Workspace>
      </div>
    );
  }
}

// Enforce that Application receives styling.
Application.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withStyles(styles)(Application);