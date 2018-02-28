import AppBar from 'material-ui/AppBar';
import Drawer from 'material-ui/Drawer';
import React from 'react';
import PropTypes from 'prop-types';
import Toolbar from 'material-ui/Toolbar';
import Workspace from './workspace';
import { withStyles } from 'material-ui/styles';

const drawerWidth = 240;
const styles = theme => ({
  root: {
    flexGrow: 1,
    zIndex: 1,
    overflow: 'hidden',
    position: 'relative',
    display: 'flex',
  },
  appBar: {
    zIndex: theme.zIndex.drawer + 1,
  },
  drawerPaper: {
    position: 'relative',
    width: drawerWidth,
    backgroundColor: '#efe',
    padding: '10px',
  },
  content: {
    flexGrow: 1,
    minWidth: 0,
    backgroundColor: '#eef',
    padding: '10px',
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
            Menu Here.
          </Toolbar>
        </AppBar>
        <Drawer variant='permanent' 
                classes={{ paper: classes.drawerPaper }}>
          { /* Add div to account for menu bar */ }
          <div className={classes.toolbar} />
          Drawer Content here.
        </Drawer>
        <Workspace className={classes.content}>
            { /* Add div to account for menu bar */ }
            <div className={classes.toolbar} />
            Insert Application Here.
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